#include "plotviewer.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "sessionmanager.h"
#include "xmlaccessor.h"
#include "enumclasses.h"
#include "easyNetMainWindow.h"
#include "lazynutjob.h"
#include "plotwindow.h"
#include "xmlform.h"
#include "objectnamevalidator.h"
#include "xmlelement.h"



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
#include <QCheckBox>
#include <QDebug>
#include <QKeyEvent>
#include <QPushButton>
#include <QGridLayout>
#include <QSpacerItem>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QScrollArea>


FullScreenSvgDialog::FullScreenSvgDialog(QWidget *parent)
    :QDialog(parent, Qt::FramelessWindowHint)
{
    svg = new QSvgWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    QGridLayout *glayout = new QGridLayout;
    QPushButton *closeBtn = new QPushButton("Close");
//    glayout->addSpacerItem();
//    glayout->addWidget(new QSpacerItem(), 0, 0, 0, 9);
//    glayout->addWidget(closeBtn, 0, 9, 0, 10);

    QHBoxLayout *hlayout = new QHBoxLayout;
    QSpacerItem *item = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
    hlayout->addSpacerItem(item);
    hlayout->addWidget(closeBtn);

    layout->addLayout(hlayout);
    layout->addWidget(svg);
    setLayout(layout);
    connect (closeBtn, SIGNAL(clicked()),this,SLOT(close()));
}

void FullScreenSvgDialog::loadByteArray(QByteArray byteArray)
{
    svg->load(byteArray);
}

void FullScreenSvgDialog::clearSvg()
{
    QByteArray byteArray;
    loadByteArray(byteArray);
}



PlotViewer::PlotViewer(QString easyNetHomei, QWidget* parent)
    : easyNetHome(easyNetHomei),
      progressiveTabIdx(0),
      fullScreen(false),
      ResultsWindow_If(parent),
      pend(false),
      askMakeSnapshot(true),
      makeSnapshot(true),
      plotClones()
{
    plotPanel = new QTabWidget;
    plotPanel->setTabsClosable(true);
    fullScreenSvgDialog = new FullScreenSvgDialog(this);
    auto temp=QApplication::desktop()->availableGeometry();
    fullScreenSize = QSize(temp.width(),temp.height());
    fullScreenSvgDialog->resize(fullScreenSize);
    setCentralWidget(plotPanel);
    createActions();
    createToolBars();

    connect(plotPanel, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
    connect(plotPanel, &QTabWidget::tabCloseRequested, [=](int index)
    {
        QSvgWidget* svg = qobject_cast<QSvgWidget*>(plotPanel->widget(index));
        QString name = plotSvg.key(svg);
        if (!svg || name.isEmpty())
            return;
        if (svgIsActive.value(svg))
        {
            if (askMakeSnapshot)
            {
                makeSnapshot = makeSnapshotMsg->exec() == QMessageBox::Yes;
                askMakeSnapshot = dontAskAgainMakeSnapshotCheckBox->checkState() == Qt::Unchecked;
            }
            if (makeSnapshot)
                snapshot(name);
        }

        deletePlot(name);
    });

    resizeTimer = new QTimer(this);
    connect(resizeTimer,SIGNAL(timeout()),this,SLOT(resizeTimeout()));

    dataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    connect(dataframeFilter, &ObjectCacheFilter::objectModified, [=](QString df)
    {
        foreach (QString plot, sourceDataframeOfPlots.values(df))
        {
            QSvgWidget* svg = plotSvg.value(plot);
            if (svgIsActive.value(svg))
            {
                svgSourceModified[svg] = true;
                updateActivePlots();
            }
        }
    });
    plotFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    plotFilter->setFilterKeyColumn(ObjectCache::NameCol);
    plotUpdater = new ObjectUpdater(this);
    plotUpdater->setProxyModel(plotFilter);
    connect(plotFilter, SIGNAL(objectCreated(QString,QString,QDomDocument*)),
            this, SLOT(generatePrettyName(QString,QString,QDomDocument*)));
    connect(plotUpdater, &ObjectUpdater::objectUpdated, [=](QDomDocument* domDoc, QString plotName)
    {
        QString prettyName =  XMLelement(*domDoc)["pretty name"]();
        plotPanel->setTabText(plotPanel->indexOf(plotSvg.value(plotName)), prettyName);
    });

    validator = new ObjectNameValidator(this);
    setSingleTrialMode(Dispatch_New);
    setTrialListMode(Dispatch_New);

    makeSnapshotMsg = new QMessageBox(
                QMessageBox::Question,
                "Make a snapshot",
                "Would you like to create a snapshot of the current plot instead of deleting it entirely?\n"
                "A snapshot can be saved as SVG, but cannot be modified.",
                QMessageBox::Yes | QMessageBox::No,
                this);
    dontAskAgainMakeSnapshotCheckBox = new QCheckBox("don't show this message again");
    makeSnapshotMsg->setCheckBox(dontAskAgainMakeSnapshotCheckBox);
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

    editToolBar->addAction(snapshotAct);
    editToolBar->addAction(settingsAct);
    editToolBar->addAction(fullScreenAct);
//    editToolBar->addAction(renameAct);
//    titleLabel = new QLabel("");
    dispatchToolBar->removeAction(setDispatchModeOverrideActs.at(Dispatch_Append));
}

void PlotViewer::dispatch_Impl(QDomDocument *info)
{
    QMutableMapIterator<QSvgWidget*, bool> svgSourceModified_it(svgSourceModified);
    while (svgSourceModified_it.hasNext())
    {
        svgSourceModified_it.next();
        if (svgSourceModified_it.value())
        {
            svgSourceModified_it.setValue(false);
            QSvgWidget* svg = svgSourceModified_it.key();
            svgIsUpToDate[svg] = false;
            svgTrialRunInfo[svg] = info;
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
    QSet<QString> affectedPlots;
    QSet<QString> sourceDfs({results});
    sourceDfs.unite(matchListFromMap(dataframeMergeOfSource, results));
//    if (!dataframeMergeOfSource.values(results).isEmpty())
//        sourceDfs.append(dataframeMergeOfSource.values(results));
    foreach(QString df, sourceDfs)
        foreach (QString rplot, sourceDataframeOfPlots.values(df))
            affectedPlots.insert(rplot);
    QMapIterator<QString, bool> anyTrialPlot_it(anyTrialPlot);
    while (anyTrialPlot_it.hasNext())
    {
        anyTrialPlot_it.next();
        if (anyTrialPlot_it.value())
            affectedPlots.insert(anyTrialPlot_it.key());
    }

    foreach (QString rplot, affectedPlots)
    {
        QSvgWidget* svg = plotSvg[rplot];
        int action;
        if (!svgByteArray.contains(svg))
        {
            action = Dispatch_Overwrite;
        }
        else if (!dispatchModeAuto && dispatchModeOverride > -1 && svg == currentSvgWidget())
        {
            action = dispatchModeOverride;
        }
        else if (svgDispatchOverride.value(svg) > -1)
        {
            action = svgDispatchOverride.value(svg);
        }
        else if (!svgTrialRunInfo.value(svg))
        {
            action = currentDispatchMode;
        }
        else
        {
            QDomElement previousRoot = svgTrialRunInfo[svg]->documentElement();
            QDomElement previousRunModeElement = XMLAccessor::childElement(previousRoot, "Run mode");
            QString previousRunMode = XMLAccessor::value(previousRunModeElement);
            int previousDispatchMode;
            if (previousRunMode == "single")
                previousDispatchMode = singleTrialDispatchMode;
            else if (previousRunMode == "list")
                previousDispatchMode = trialListDispatchMode;
            else
            {
                qDebug() << "ERROR: PlotViewer::preDispatch cannot read trial run info XML.";
                return;
            }
            action = dispatchModeFST.value(qMakePair(previousDispatchMode,
                                                     svgDispatchOverride.contains(svg) ? svgDispatchOverride.value(svg) : currentDispatchMode));
        }

        if (action == Dispatch_New)
        {
            QString newName = cloneRPlot(plotSvg.key(svg));
            svgIsUpToDate[plotSvg[newName]] = false;
        }
    }

//    updateActivePlots();
//    refreshInfo();
}



void PlotViewer::showInfo(QSvgWidget *svg)
{
    if (!svgTrialRunInfo.contains(svg) || !svgTrialRunInfo[svg])
    {
        return;
    }
    XMLForm *infoForm = new XMLForm(svgTrialRunInfo[svg]->documentElement());
    infoForm->build();
    infoScroll->setWidget(infoForm);
    infoForm->show();
    // leakage

}

void PlotViewer::createActions()
{
    ResultsWindow_If::createActions();
    setDispatchModeAutoAct->setEnabled(false);
    infoAct->setEnabled(false);

    openAct->setStatusTip(tr("Load plot"));
    saveAct->setStatusTip(tr("Save plot"));
    saveAct->setEnabled(false);
    copyAct->setStatusTip(tr("Copy plot to clipboard"));
    copyAct->setEnabled(false);

    settingsAct = new QAction(QIcon(":/images/plot_settings.png"), tr("&Settings"), this);
    settingsAct->setShortcut(QKeySequence::Refresh);
    settingsAct->setStatusTip(tr("Plot settings"));
    connect(settingsAct, SIGNAL(triggered()), this, SIGNAL(showPlotSettings()));
    settingsAct->setEnabled(false);

    refreshAct = new QAction(QIcon(":/images/refresh.png"), tr("&Refresh"), this);
    refreshAct->setShortcut(QKeySequence::Refresh);
    refreshAct->setStatusTip(tr("Refresh plot"));
    connect(refreshAct, &QAction::triggered, [=](){
       emit sendDrawCmd(plotSvg.key(currentSvgWidget()));
    });

    snapshotAct = new QAction(QIcon(":/images/snapshot-icon.png"), tr("Snapshot"), this);
    snapshotAct->setStatusTip(tr("Snapshot"));
    connect(snapshotAct, SIGNAL(triggered()), this, SLOT(snapshot()));
    snapshotAct->setEnabled(false);

    renameAct =  new QAction(QIcon(":/images/rename-icon.png"), tr("Rename"), this);
    renameAct->setStatusTip(tr("Rename"));
    connect(renameAct, SIGNAL(triggered()), this, SLOT(renamePlot()));


    fullScreenAct = new QAction(QIcon(":/images/Full_screen_view.png"), "Full Screen", this);
    connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(setupFullScreen()));
    fullScreenAct->setEnabled(false);
//    setDispatchModeOverrideActs.at(Dispatch_Append)->setDisabled(true);

}

void PlotViewer::open()
{
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load SVG File"),
                                                    easyNetHome,
                                                    tr("SVG Files (*.svg)"));
    if (!fileName.isEmpty())
    {
        QString name = uniqueName(QFileInfo(fileName).completeBaseName());
        QSvgWidget* svg = newSvg(name);
        setSvgActive(false, svg);
        svg->load(fileName);
        plotPanel->setCurrentWidget(svg);
        plotPanel->setTabText(plotPanel->currentIndex(), name);
        currentTabChanged(plotPanel->currentIndex()); // refresh action enable properties
    }
}

void PlotViewer::loadByteArray(QString name, QByteArray byteArray)
{
    if (fullScreen)
        fullScreenSvgDialog->loadByteArray(byteArray);
    else
    {
        QSvgWidget* svg = plotSvg.value(name, nullptr);
        if (svg)
        {
            svgIsUpToDate[svg] = true;
            svgByteArray[svg] = byteArray;
            svg->load(byteArray);
            plotPanel->setCurrentWidget(svg);
//            titleLabel->setText(name);
            setTabState(plotPanel->indexOf(svg) ,Tab_Ready);
        }
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

void PlotViewer::newRPlot(QString name, QString type, QMap<QString, QString> defaultSettings, QMap<QString, QString> sourceDataframeSettings, bool anyTrial, int dispatchOverride, QDomDocument *info)
{
    Q_UNUSED(defaultSettings)
    if (plotSvg.contains(name))
    {
        snapshot(name);
        deletePlot(name);
    }

    plotType.insert(name, type);
    anyTrialPlot.insert(name, anyTrial);
    addSourceDataframes(name, sourceDataframeSettings);
//    plotSourceDataframeSettings.insert(name, sourceDataframeSettings);
//    foreach (QString df, sourceDataframeSettings.values())
//    {
//        if (!sourceDataframeOfPlots.contains(df, name))
//            sourceDataframeOfPlots.insert(df, name);
//        dataframeFilter->addName(df);
//    }
    QSvgWidget* svg = newSvg(name);
    svgDispatchOverride.insert(svg, dispatchOverride);
    svgTrialRunInfo.insert(svg, info);
    plotFilter->addName(name);

}


void PlotViewer::updateActivePlots()
{
    QMapIterator<QSvgWidget*, bool> plotIsActiveIt(svgIsActive);
    while (plotIsActiveIt.hasNext())
    {
        plotIsActiveIt.next();
        if (plotIsActiveIt.value())
        {
            if (svgIsUpToDate[plotIsActiveIt.key()])
                setTabState(plotPanel->indexOf(plotIsActiveIt.key()), Tab_DefaultState);
            else
                setTabState(plotPanel->indexOf(plotIsActiveIt.key()), Tab_Old);
        }
    }

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
            setTabState(plotPanel->indexOf(currentSvgWidget()), Tab_Updating);
        }
    }
}

QString PlotViewer::plotCloneName(QString name)
{
    return validator->makeValid(name);
}


QString PlotViewer::cloneRPlot(QString name, QString newName)
{
    newName =  validator->makeValid(newName.isEmpty() ? name : newName);
//    plotFilter->addName(newName);
    // set df-related settings to values that are names of copies of the original df's
    QMap<QString, QString> sourceDataframeSettings = plotSourceDataframeSettings.value(name);
    QMutableMapIterator<QString, QString>sourceDataframeSettings_it(sourceDataframeSettings);
    while(sourceDataframeSettings_it.hasNext())
    {
        sourceDataframeSettings_it.next();
        sourceDataframeSettings_it.setValue(validator->makeValid(sourceDataframeSettings_it.value()));
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
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->recentlyCreatedJob();
    QMap<QString, QVariant> jobData;
    jobData.insert("plotName", newName);
    jobData.insert("sourceDataframeSettings", QVariant::fromValue(sourceDataframeSettings));
    jobs.last()->data = jobData;
    jobs.last()->appendEndOfJobReceiver(this, SLOT(addSourceDataframes()));
    SessionManager::instance()->submitJobs(jobs);
    // have a plot settings form created for the clone rplot
    // use the current settings from the original rplot, overwrite the df related ones and set them as defaults
    // for the new form
    QMap<QString, QString> settings = MainWindow::instance()->plotSettingsWindow->getSettings(name); // bad design but quicker than using signals/slots
    sourceDataframeSettings_it.toFront();
    while(sourceDataframeSettings_it.hasNext())
    {
        sourceDataframeSettings_it.next();
        settings[sourceDataframeSettings_it.key()] = sourceDataframeSettings_it.value();
    }
    emit quietlyCreateNewRPlot(newName, plotType[name], settings, QMap<QString, QString>(), false, -1);
    // new svg for the clone plot, copy the original trial run info to the clone
//    QSvgWidget* svg = newSvg(newName);
//    svgTrialRunInfo.insert(svg, svgTrialRunInfo[plotSvg[name]]);
    newRPlot(newName, plotType[name], QMap<QString, QString>(), QMap<QString, QString>(), // sourceDataframeSettings will be set after df creation
             false, -1, svgTrialRunInfo[plotSvg[name]]);
    plotClones.append(newName);
    return newName;
}

void PlotViewer::updateAllActivePlots()
{
    QMapIterator<QSvgWidget*, bool> plotIsActiveIt(svgIsActive);
    while (plotIsActiveIt.hasNext()) {
        plotIsActiveIt.next();
        if (plotIsActiveIt.value())
        {
            svgIsUpToDate[plotIsActiveIt.key()]=false;
//            setTabState(plotPanel->indexOf(plotIsActiveIt.key()), Tab_Old);
        }
    }
    updateActivePlots();
}

void PlotViewer::setTabState(int index, int state)
{
    QIcon icon;
    switch(state)
    {
    case Tab_DefaultState:
        icon = QIcon();
        break;
    case Tab_Updating:
        icon = QIcon(":/images/view_refresh.png");
        break;
    case Tab_Ready:
        icon = QIcon(":/images/icon_check_x24green.png");
        break;
    case Tab_Old:
        icon = QIcon(":/images/large-yellow-dot.png");
        break;
    default:
        break;
    }
    plotPanel->setTabIcon(index, icon);
}

void PlotViewer::setCurrentPlot(QString name)
{
    plotPanel->setCurrentWidget(plotSvg.value(name));
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
                          isActive ? QIcon() :
                                     QIcon(":/images/snapshot-icon.png"));
}

QSvgWidget *PlotViewer::newSvg(QString name)
{
    QSvgWidget* svg = new QSvgWidget;
    plotSvg[name] = svg;
    plotPanel->addTab(svg, "");
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
//    newName = newName.isEmpty() ? plotCloneName(oldName) : newName;

//    plotSvg[newName] = plotSvg.value(oldName);
//    plotSvg.remove(oldName);
//    if (plotSvg[newName] == currentSvgWidget())
//        titleLabel->setText(newName);
}

void PlotViewer::deletePlot(QString name)
{
    // remove all map entries, lazyNut object and svg
    QSvgWidget* svg = plotSvg.value(name);
    plotSvg.remove(name);
    plotType.remove(name);
    anyTrialPlot.remove(name);
    SessionManager::instance()->destroyObject(name);
    if (plotClones.contains(name))
    {
        foreach (QString df, sourceDataframeOfPlots.keys(name))
            SessionManager::instance()->destroyObject(df);
        plotClones.removeAll(name);
    }

    svgIsActive.remove(svg);
    svgByteArray.remove(svg);
    svgIsUpToDate.remove(svg);
    svgSourceModified.remove(svg);
    svgTrialRunInfo.remove(svg); // leakage, but smart pointers will fix
    svgDispatchOverride.remove(svg);

    plotSourceDataframeSettings.remove(name);
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

    delete svg;
    emit removePlot(name);

    if (plotPanel->count() == 0)
    {
        settingsAct->setEnabled(false);
        snapshotAct->setEnabled(false);
        saveAct->setEnabled(false);
        copyAct->setEnabled(false);
        fullScreenAct->setEnabled(false);
        infoAct->setEnabled(false);
        setDispatchModeAutoAct->setEnabled(false);
    }

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
    if (!dataframeMergeOfSource.contains(df, dfm))
        dataframeMergeOfSource.insert(df, dfm);
}


void PlotViewer::setupFullScreen()
{
    fullScreen = true;
    emit resized(fullScreenSize);
    emit sendDrawCmd(plotSvg.key(currentSvgWidget()));
    fullScreenSvgDialog->clearSvg();
    fullScreenSvgDialog->exec();
    fullScreen = false;
}

void PlotViewer::generatePrettyName(QString plotName, QString type, QDomDocument *domDoc)
{
    Q_UNUSED(type)
    Q_UNUSED(domDoc)
    QString prettyName = plotName;
    prettyName.remove(".plot");
    prettyName.replace(".", " ");
    SessionManager::instance()->setPrettyName(plotName, prettyName);
}

void PlotViewer::addSourceDataframes(QString plotName, QMap<QString, QString> sourceDataframeSettings)
{
    if (plotName.isEmpty())
    {
        QVariant pNVariant = SessionManager::instance()->getDataFromJob(sender(), "plotName");
        if (!pNVariant.canConvert<QString>())
        {
            qDebug() << "ERROR: PlotViewer::addDataframeToFilter cannot retrieve a valid string from plotName key in sender LazyNut job";
            return;
        }
        plotName = pNVariant.toString();
        if (plotName.isEmpty())
        {
            qDebug() << "ERROR: PlotViewer::addDataframeToFilter string from plotName key in sender LazyNut job is empty";
            return;
        }
        QVariant sDFVariant = SessionManager::instance()->getDataFromJob(sender(), "sourceDataframeSettings");
        if (!sDFVariant.canConvert<QMap<QString, QString> >())
        {
            qDebug() << "ERROR: PlotViewer::addDataframeToFilter cannot retrieve a valid string from sourceDataframeSettings key in sender LazyNut job";
            return;
        }
        sourceDataframeSettings = sDFVariant.value<QMap<QString, QString> >();
        if (sourceDataframeSettings.isEmpty())
        {
             qDebug() << "ERROR: PlotViewer::addDataframeToFilter string from sourceDataframeSettings key in sender LazyNut job is empty";
             return;
        }
    }
    plotSourceDataframeSettings.insert(plotName, sourceDataframeSettings);
    foreach(QString df, sourceDataframeSettings.values())
    {
        if (!sourceDataframeOfPlots.contains(df, plotName))
            sourceDataframeOfPlots.insert(df, plotName);
        if (!SessionManager::instance()->descriptionCache->exists(df))
        {
            qDebug() << QString("ERROR: PlotViewer::addDataframeToFilter attempt to add a non-existing dataframe %1")
                        .arg(df);
        }
        else
        {
            dataframeFilter->addName(df);
        }
    }
}

QString PlotViewer::uniqueName(QString name)
{
    if (!plotSvg.contains(name))
        return name;
    int progNum = 1;
    QString newName = QString("%1.%2").arg(name).arg(QString::number(progNum));
    while (plotSvg.contains(newName))
    {
        progNum++;
        newName = QString("%1.%2").arg(name).arg(QString::number(progNum));
    }

    return newName;
}

void PlotViewer::snapshot(QString name)
{
    QSvgWidget* svg;
    if (name.isEmpty())
    {
        svg = currentSvgWidget();
        name = plotSvg.key(svg);
    }
    else
    {
        svg = plotSvg.value(name);
    }
    if (!svg)
        return;
    QString snapshotName = uniqueName(name);
    QSvgWidget* snapshotSvg = newSvg(snapshotName);
    plotPanel->setTabText(plotPanel->indexOf(snapshotSvg), snapshotName);
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
    setDispatchModeAutoAct->setEnabled(svgIsActive.value(svg));
    saveAct->setEnabled(true);
    copyAct->setEnabled(true);
    fullScreenAct->setEnabled(true);
    infoAct->setEnabled(true);
//    titleLabel->setText(plotSvg.key(svg));
}

void PlotViewer::currentTabChanged(int index)
{
    QSvgWidget* svg = static_cast<QSvgWidget*>(plotPanel->widget(index));
    emit setPlot(plotSvg.key(svg));
    updateActivePlots();
    updateActionEnabledState(svg);
    if (infoVisible)
        showInfo(svg);
}
