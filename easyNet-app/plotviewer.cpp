#include "plotviewer.h"
#include "objectcachefilter.h"
#include "sessionmanager.h"
#include "xmlaccessor.h"
#include "enumclasses.h"
#include "easyNetMainWindow.h"
#include "lazynutjob.h"
#include "plotwindow.h"
#include "xmlform.h"

#include <QSvgWidget>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QFile>
#include <QClipboard>
#include <QApplication>
#include <QPixmap>
#include <QInputDialog>
#include <QDomDocument>
#include <QMessageBox>
#include <QDebug>

PlotViewer::PlotViewer(QString easyNetHome, QWidget* parent)
    : easyNetHome(easyNetHome), progressiveTabIdx(0), pend(false), ResultsWindow_If(parent)
{
    plotPanel = new QTabWidget;
    setCentralWidget(plotPanel);
    createActions();
    createToolBars();

    connect(plotPanel, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

    resizeTimer = new QTimer(this);
    connect(resizeTimer,SIGNAL(timeout()),this,SLOT(resizeTimeout()));

    dataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    connect(dataframeFilter, &ObjectCacheFilter::objectModified, [=](QString df)
    {
        foreach (QString plot, sourceDataframeOfPlots.values(df))
        {
//            qDebug() << "PlotViewer objectModified" << df << plot;
            QSvgWidget* svg = plotSvg.value(plot);
            if (svgIsActive.value(svg))
            {
                svgSourceModified[svg] = true;
//                qDebug() << "plotSourceModified" << plot;
            }
        }
    });
    setSingleTrialMode(Dispatch_New);
    setTrialListMode(Dispatch_New);

}

PlotViewer::~PlotViewer()
{

}

QSvgWidget *PlotViewer::currentSvgWidget()
{
    return qobject_cast<QSvgWidget*>(plotPanel->currentWidget());
}

void PlotViewer::createToolBars()
{
    ResultsWindow_If::createToolBars();

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(refreshAct);
    editToolBar->addAction(snapshotAct);
    editToolBar->addAction(settingsAct);

    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(renameAct);
    fileToolBar->addAction(copyAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(deleteAct);

    titleLabel = new QLabel("");
    navigationToolBar = addToolBar(tr("Plots"));
    navigationToolBar->addSeparator();
    navigationToolBar->addWidget(titleLabel);
}

void PlotViewer::dispatch_Impl(QDomDocument *info)
{
    Q_UNUSED(info)
    QMutableMapIterator<QSvgWidget*, bool> svgSourceModified_it(svgSourceModified);
    while (svgSourceModified_it.hasNext())
    {
        svgSourceModified_it.next();
        if (svgSourceModified_it.value())
        {
            svgSourceModified_it.setValue(false);
            QSvgWidget* svg = svgSourceModified_it.key();
            svgIsUpToDate[svg] = false;
            svgTrialRunInfo[svg].append(info);
        }
    }
    updateActivePlots();
    refreshInfo();
}

void PlotViewer::preDispatch(QDomDocument *info)
{
    QDomElement rootElement = info->documentElement();
    QDomElement runModeElement = XMLAccessor::childElement(rootElement, "Run mode");
    QString runMode = XMLAccessor::value(runModeElement);
    QDomElement resultsElement = XMLAccessor::childElement(rootElement, "Results");
    QString results = XMLAccessor::value(resultsElement);

    int currentDispatchMode;
    if (runMode == "single")
        currentDispatchMode = singleTrialDispatchMode;
    else if (runMode == "list")
        currentDispatchMode = trialListDispatchMode;
    else
    {
        qDebug() << "ERROR: PlotViewer::dispatch_private cannot read trial run info XML.";
        return;
    }
    int action;
    QStringList sourceDfs({results});
    if (!dataframeMergeOfSource.values(results).isEmpty())
        sourceDfs.append(dataframeMergeOfSource.values(results));
    qDebug() << " PlotViewer::preDispatch sourceDfs" << sourceDfs;
    foreach (QString df, sourceDfs)
    {
        foreach (QString rplot, sourceDataframeOfPlots.values(df))
        {
            QSvgWidget* svg = plotSvg[rplot];
            if (!dispatchModeAuto && svg == currentSvgWidget()) // apply manual override only to visible tab
                action = dispatchModeOverride > -1 ? dispatchModeOverride : currentDispatchMode;
            else
                action = svgDispatchOverride.value(svg) > -1 ? svgDispatchOverride.value(svg) : currentDispatchMode;
            action = svgByteArray.contains(svg) ? action : Dispatch_Overwrite; // if tab is empty, write on it anyway
            qDebug() << " PlotViewer::preDispatch rplot action" << rplot << action;
            if (action == Dispatch_New)
            {
                QString newName = cloneRPlot(plotSvg.key(svg));
                svgIsUpToDate[plotSvg[newName]] = false;
            }
        }
    }
    updateActivePlots();
    refreshInfo();
}


void PlotViewer::dispatch_private(QDomDocument *info, bool preDispatch)
{
    // not used
    QDomElement rootElement = info->documentElement();
    QDomElement runModeElement = XMLAccessor::childElement(rootElement, "Run mode");
    QString runMode = XMLAccessor::value(runModeElement);

    int currentDispatchMode;
    if (runMode == "single")
        currentDispatchMode = singleTrialDispatchMode;
    else if (runMode == "list")
        currentDispatchMode = trialListDispatchMode;
    else
    {
        qDebug() << "ERROR: PlotViewer::dispatch_private cannot read trial run info XML.";
        return;
    }
    int action;
    QMutableMapIterator<QSvgWidget*, bool> svgSourceModified_it(svgSourceModified);
    while (svgSourceModified_it.hasNext())
    {
        svgSourceModified_it.next();
        if (svgSourceModified_it.value())
        {
            svgSourceModified_it.setValue(false);
            QSvgWidget* svg = svgSourceModified_it.key();
            if (!dispatchModeAuto && svg == currentSvgWidget()) // apply manual override only to visible tab
                action = dispatchModeOverride > -1 ? dispatchModeOverride : currentDispatchMode;
            else
                action = svgDispatchOverride.value(svg) > -1 ? svgDispatchOverride.value(svg) : currentDispatchMode;
            action = svgByteArray.contains(svg) ? action : Dispatch_Overwrite; // if tab is empty, write on it anyway

            if (preDispatch && action == Dispatch_New)
            {
                QString newName = cloneRPlot(plotSvg.key(svg));
                svgIsUpToDate[plotSvg[newName]] = false;
            }
            else
            {
                svgIsUpToDate[svg] = false;
                svgTrialRunInfo[svg].append(info);
            }
//            switch(action)
//            {
//            case Dispatch_New:
//            {
//                cloneRPlot(plotSvg.key(svg));
//                // same as Dispatch_Overwrite
//                svgIsUpToDate[svg] = false;
//                svgTrialRunInfo[svg].append(info);
//                break;
//            }
//            case Dispatch_Append: // not implemented for the moment, take it as overwrite
//            case Dispatch_Overwrite:
//            {
//                svgIsUpToDate[svg] = false;
//                svgTrialRunInfo[svg].append(info);
//                break;
//            }
//            default:
//            {
//                qDebug() << "PlotViewer::dispatch_Impl computed action was unrecognised";
//                return;
//            }
//            }
        }
    }
    updateActivePlots();
    refreshInfo();
}

void PlotViewer::showInfo(QSvgWidget *svg)
{
    if (!svgTrialRunInfo.contains(svg) || svgTrialRunInfo.value(svg).isEmpty())
    {
        return;
    }
    XMLForm *infoForm = new XMLForm(svgTrialRunInfo[svg].last()->documentElement());
    infoForm->build();
    infoScroll->setWidget(infoForm);
    infoForm->show();
    // leakage

}

void PlotViewer::createActions()
{
    ResultsWindow_If::createActions();

    openAct->setStatusTip(tr("Load plot"));
    saveAct->setStatusTip(tr("Save plot"));
    copyAct->setStatusTip(tr("Copy plot to clipboard"));


    settingsAct = new QAction(QIcon(":/images/plot_settings.png"), tr("&Settings"), this);
    settingsAct->setShortcut(QKeySequence::Refresh);
    settingsAct->setStatusTip(tr("Plot settings"));
    connect(settingsAct, SIGNAL(triggered()), this, SIGNAL(showPlotSettings()));

    refreshAct = new QAction(QIcon(":/images/refresh.png"), tr("&Refresh"), this);
    refreshAct->setShortcut(QKeySequence::Refresh);
    refreshAct->setStatusTip(tr("Refresh plot"));
    connect(refreshAct, &QAction::triggered, [=](){
       emit sendDrawCmd(plotSvg.key(currentSvgWidget()));
    });

    snapshotAct = new QAction(QIcon(":/images/snapshot-icon.png"), tr("Snapshot"), this);
    snapshotAct->setStatusTip(tr("Snapshot"));
    connect(snapshotAct, SIGNAL(triggered()), this, SLOT(snapshot()));

    renameAct =  new QAction(QIcon(":/images/rename-icon.png"), tr("Rename"), this);
    renameAct->setStatusTip(tr("Rename"));
    connect(renameAct, SIGNAL(triggered()), this, SLOT(renamePlot()));

    deleteAct = new QAction(QIcon(":/images/delete-icon.png"), tr("&Delete"), this);
    deleteAct->setShortcut(QKeySequence::Delete);
    deleteAct->setStatusTip(tr("Delete plot"));
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(deletePlot()));
}

void PlotViewer::open()
{
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load SVG File"),
                                                    easyNetHome,
                                                    tr("SVG Files (*.svg)"));
    if (!fileName.isEmpty())
    {
        QString name = QFileInfo(fileName).baseName();
        if (plotSvg.contains(name))
            name = QString("%1_tab%2").arg(name).arg(QString::number(plotCloneCount[name]++));

        QSvgWidget* svg = newSvg(name);
        setSvgActive(false, svg);
        svg->load(fileName);
        plotPanel->setCurrentWidget(svg);
        currentTabChanged(plotPanel->currentIndex()); // refresh action enable properties
    }
}

void PlotViewer::loadByteArray(QString name, QByteArray byteArray)
{
    QSvgWidget* svg = plotSvg.value(name, nullptr);
    if (svg)
    {
        svgIsUpToDate[svg] = true;
        svgByteArray[svg] = byteArray;
        svg->load(byteArray);
        plotPanel->setCurrentWidget(svg);
        titleLabel->setText(name);
    }
}

void PlotViewer::save()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save SVG File"),
                                                    QString("%1/%2").arg(easyNetHome)
                                                    .arg(plotSvg.key(currentSvgWidget())),
                                                    tr("SVG Files (*.svg)"));
    if (!fileName.isEmpty())
    {
          QFile file(fileName);
          file.open(QIODevice::WriteOnly);
          file.write(svgByteArray.value(currentSvgWidget()));
          file.close();
    }
}

void PlotViewer::copy()
{
    QPixmap* currPixmap = new QPixmap(currentSvgWidget()->width(),
                                      currentSvgWidget()->height());
    currentSvgWidget()->render(currPixmap);
    QClipboard *p_Clipboard = QApplication::clipboard();
    p_Clipboard->setPixmap(*currPixmap);
}

void PlotViewer::setInfoVisible(bool visible)
{
    infoVisible = visible;
    if (infoVisible)
        showInfo(currentSvgWidget());
}

void PlotViewer::refreshInfo()
{
    if (infoVisible)
        showInfo(currentSvgWidget());
}

void PlotViewer::newRPlot(QString name, QString type, QMap<QString, QString> defaultSettings, QMap<QString, QString> sourceDataframeSettings, int dispatchOverride)
{
    Q_UNUSED(defaultSettings)

    plotType.insert(name, type);
    plotSourceDataframeSettings.insert(name, sourceDataframeSettings);
    foreach (QString df, sourceDataframeSettings.values())
    {
        if (!sourceDataframeOfPlots.contains(df, name))
            sourceDataframeOfPlots.insert(df, name);
        if (!dataframeCloneCount.contains(df))
            dataframeCloneCount.insert(df, 0);
        dataframeFilter->addName(df);
    }
    plotCloneCount.insert(name, 0);
    QSvgWidget* svg = newSvg(name);
    svgDispatchOverride.insert(svg, dispatchOverride);

}

//void PlotViewer::addPlot(QString name, QString sourceDataframe)
//{
////    QSvgWidget *svg = new QSvgWidget;
////    plotName[svg] = name;
//    if (!sourceDataframe.isEmpty() && !plotDataframeMap.contains(sourceDataframe))
//        dataframeFilter->addName(sourceDataframe);

//    if (!sourceDataframe.isEmpty())
//        plotDataframeMap[sourceDataframe].insert(name);

//    newSvg(name);

////    plotPanel->addTab(svg, QString("Plot %1").arg(++progressiveTabIdx));
////    setPlotActive(true, svg);
////    plotIsUpToDate[svg] = false;
////    plotPanel->setCurrentWidget(svg);
////    currentTabChanged(plotPanel->currentIndex());
////    return svg;
//}

void PlotViewer::updateActivePlots()
{
    if(plotPanel->currentIndex()>-1 &&
            svgIsActive[currentSvgWidget()] &&
            !svgIsUpToDate[currentSvgWidget()])
    {
        if(visibleRegion().isEmpty())
        {
            pend=true;
        }
        else
        {
            emit sendDrawCmd(plotSvg.key(currentSvgWidget()));
        }
    }
}

QString PlotViewer::plotCloneName(QString name)
{
    return QString("%1_tab%2").arg(normalisedName(name)).arg(QString::number(plotCloneCount[name]++));
}

QString PlotViewer::normalisedName(QString name)
{
    name.replace(QRegExp("[()]"), "");
    name.replace(" ", "_");
    return name;
}

QString PlotViewer::cloneRPlot(QString name, QString newName)
{
    newName = newName.isEmpty() ? plotCloneName(name) : newName;
    // set df-related settings to values that are names of copies of the original df's
    QMap<QString, QString> sourceDataframeSettings = plotSourceDataframeSettings.value(name);
    QMutableMapIterator<QString, QString>sourceDataframeSettings_it(sourceDataframeSettings);
    while(sourceDataframeSettings_it.hasNext())
    {
        sourceDataframeSettings_it.next();
        sourceDataframeSettings_it.setValue(QString("%1_rplotCopy%2")
                                            .arg(normalisedName(sourceDataframeSettings_it.value()))
                                            .arg(dataframeCloneCount[sourceDataframeSettings_it.value()]++));
    }
    // make the above mentioned df copies
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER; // debug purpose
    sourceDataframeSettings_it.toFront();
    while(sourceDataframeSettings_it.hasNext())
    {
        sourceDataframeSettings_it.next();
        job->cmdList.append(QString("%1 copy %2")
                            .arg(plotSourceDataframeSettings.value(name).value(sourceDataframeSettings_it.key()))
                            .arg(sourceDataframeSettings_it.value()));
    }
    job->data = QVariant::fromValue(QMap<QString, QString>({{"plotName", newName}}));
    job->appendEndOfJobReceiver(this, SLOT(triggerPlotUpdate()));
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->recentlyCreatedJob();
    SessionManager::instance()->submitJobs(jobs);
    // have a plot settings form created for the clone rplot
    // use the current settings from the original rplot, overwrite the df related ones and set them as defaults
    // for the new form
    QMap<QString, QString> settings = MainWindow::instance()->plotSettingsWindow->getSettings(name); // bad design but quicker than using sinals/slots
    sourceDataframeSettings_it.toFront();
    while(sourceDataframeSettings_it.hasNext())
    {
        sourceDataframeSettings_it.next();
        settings[sourceDataframeSettings_it.key()] = sourceDataframeSettings_it.value();
    }
    emit createNewRPlot(newName, plotType[name], settings, QMap<QString, QString>(), -1);
    // new svg for the clone plot, copy the original trial run info to the clone
    QSvgWidget* svg = newSvg(newName);
    svgTrialRunInfo.insert(svg, svgTrialRunInfo[plotSvg[name]]);

    return newName;
}

void PlotViewer::updateAllActivePlots()
{
    QMapIterator<QSvgWidget*, bool> plotIsActiveIt(svgIsActive);
    while (plotIsActiveIt.hasNext()) {
        plotIsActiveIt.next();
        if (plotIsActiveIt.value())
            svgIsUpToDate[plotIsActiveIt.key()]=false;
    }
    updateActivePlots();
}

void PlotViewer::paintEvent(QPaintEvent * event)
{
    if(pend)
    {
        pend=false;
        resizeTimer->stop();
        resizeTimer->start(250);
    }
}

void PlotViewer::resizeEvent(QResizeEvent*)
{
    resizeTimer->stop();
    resizeTimer->start(250);
}


void PlotViewer::setSvgActive(bool isActive, QSvgWidget *svg)
{
    svg = svg ? svg : currentSvgWidget();
    svgIsActive[svg] = isActive;
    plotPanel->setTabIcon(plotPanel->indexOf(svg),
                          isActive ? QIcon(":/images/icon-record.png") :
                                     QIcon(":/images/snapshot-icon.png"));
}

QSvgWidget *PlotViewer::newSvg(QString name)
{
    QSvgWidget* svg = new QSvgWidget;
    plotSvg[name] = svg;
    plotPanel->addTab(svg, QString("Plot %1").arg(++progressiveTabIdx));
    if (svg == currentSvgWidget())
        qDebug() << "current svg changed to " << plotSvg.key(svg);
    setSvgActive(true, svg);
    svgIsUpToDate[svg] = true;
    svgSourceModified[svg] = false;
    if (svg == currentSvgWidget())
        updateActionEnabledState(svg);
    return svg;
}




void PlotViewer::resizeTimeout()
{
   resizeTimer->stop();
   emit resized(plotPanel->size());
   updateAllActivePlots();
}


void PlotViewer::freeze(QSvgWidget* svg)
{
    if (!svg)
        svg = currentSvgWidget();

    setSvgActive(false, svg);
}

void PlotViewer::renamePlot()
{
    // available only for inactive plots
    if (svgIsActive.value(currentSvgWidget(), true))
        return;
    bool ok;
    QString oldName = plotSvg.key(currentSvgWidget());
    QString newName = QInputDialog::getText(this, "Rename plot",
                                            "New name:                                    ",
                                            QLineEdit::Normal,
                                            oldName, &ok);
    if (ok && !newName.isEmpty())
    {
        if (plotSvg.contains(newName))
        {
            QMessageBox::critical(this, "Illegal plot name",QString("The name you chose is not valid, since it conflicts with an existing plot name\n"
                                                                         "Please select another name."));
            return;
        }
        renamePlot(oldName, newName);
    }
}

void PlotViewer::renamePlot(QString oldName, QString newName)
{
    newName = newName.isEmpty() ? plotCloneName(oldName) : newName;

    plotSvg[newName] = plotSvg.value(oldName);
    plotSvg.remove(oldName);
    if (plotSvg[newName] == currentSvgWidget())
        titleLabel->setText(newName);
}

void PlotViewer::deletePlot()
{
    QString name = plotSvg.key(currentSvgWidget());
    QSvgWidget* svg = currentSvgWidget();
    // remove all map entries
    plotSvg.remove(name);
    plotType.remove(name);
    plotSourceDataframeSettings.remove(name);
    plotCloneCount.remove(name);
    // remove all <df, name> pairs from sourceDataframeOfPlots
    // looks like there is no simple way to iterate mutably through a multimap
    // so first take note of all df keys associated with the plot name and then remove them
    QSet<QString> dfSet;
    QMapIterator<QString, QString> sourceDataframeOfPlots_it(sourceDataframeOfPlots);
    while (sourceDataframeOfPlots_it.hasNext())
    {
        if (sourceDataframeOfPlots.values(sourceDataframeOfPlots_it.next().key()).contains(name))
            dfSet.insert(sourceDataframeOfPlots_it.key());
    }
    foreach(QString df, dfSet)
        sourceDataframeOfPlots.remove(df, name);

    svgIsActive.remove(svg);
    svgByteArray.remove(svg);
    svgIsUpToDate.remove(svg);
    svgSourceModified.remove(svg);
    svgTrialRunInfo.remove(svg); // leakage, but smart pointers will fix
    svgDispatchOverride.remove(svg);

    delete svg;
}

void PlotViewer::makeSnapshot(QString name)
{
    qDebug() << "PlotViewer::makeSnapshot" << name;
    QSvgWidget* svg = plotSvg.value(name);
    freeze(svg);
    renamePlot(name);
}

void PlotViewer::triggerPlotUpdate(QString name)
{
    // if name is empty, first check if the sender is a LazyNut job where plotName
    // is specified, otherwise assume the current svg
    QSvgWidget *svg;
    if (name.isEmpty())
    {
        LazyNutJob *job = qobject_cast<LazyNutJob *>(sender());
        if (!job || !job->data.toMap().contains("plotName"))
            svg = currentSvgWidget();
        else
            svg = plotSvg.value(job->data.toMap().value("plotName").toString(), nullptr);
    }
    else
        svg = plotSvg.value(name, nullptr);

    if (svg)
    {
        qDebug() << "PlotViewer::triggerPlotUpdate" << name;
        svgIsUpToDate[svg] = false;
        updateActivePlots();
    }
    else
    {
        qDebug() << "ERROR: PlotViewer::triggerPlotUpdate could not find a valid svg; name was:" << name;
        return;
    }
}

void PlotViewer::addDataframeMerge(QString df, QString dfm)
{
    qDebug() << "addDataframeMerge" << df << dfm;
    if (!dataframeMergeOfSource.contains(df, dfm))
        dataframeMergeOfSource.insert(df, dfm);
}

void PlotViewer::snapshot()
{
    QSvgWidget* svg = currentSvgWidget();
    QString name = plotSvg.key(svg);
    QString snapshotName = plotCloneName(name);
    QSvgWidget* snapshotSvg = newSvg(snapshotName);
    setSvgActive(false, snapshotSvg);
    svgTrialRunInfo.insert(snapshotSvg, svgTrialRunInfo[svg]);
    snapshotSvg->load(svgByteArray[svg]);
}


void PlotViewer::updateActionEnabledState(QSvgWidget* svg)
{
    settingsAct->setEnabled(svgIsActive.value(svg));
    refreshAct->setEnabled(svgIsActive.value(svg));
    snapshotAct->setEnabled(svgIsActive.value(svg));
    renameAct->setEnabled(!svgIsActive.value(svg));
    deleteAct->setEnabled(!svgIsActive.value(svg));
    titleLabel->setText(plotSvg.key(svg));
}

void PlotViewer::currentTabChanged(int index)
{
    QSvgWidget* svg = static_cast<QSvgWidget*>(plotPanel->widget(index));
    if (svgIsActive.value(svg))
    {
        emit setPlot(plotSvg.key(svg));
        updateActivePlots();
//        if(!plotIsUpToDate[svg]&&!visibleRegion().isEmpty()) emit sendDrawCmd(plotName.value(svg));
    }
    updateActionEnabledState(svg);
    if (infoVisible)
        showInfo(svg);
}
