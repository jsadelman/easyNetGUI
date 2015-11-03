#include "plotviewer.h"
#include "objectcachefilter.h"
#include "sessionmanager.h"
#include "xmlaccessor.h"

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
        foreach (QString plot, plotDataframeMap.value(df))
        {
            QSvgWidget* svg = plotName.key(plot);
            if (plotIsActive.value(svg))
            {
                plotSourceModified[svg] = true;
            }
        }
    });
    setSingleTrialMode(Overwrite);
    setTrialListMode(New);
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
    QDomElement rootElement = info->documentElement();
    QDomElement runModeElement = XMLAccessor::childElement(rootElement, "Run mode");
    QString runMode = XMLAccessor::value(runModeElement);

    int currentDispatchMode;
    if (!dispatchModeAuto)
        currentDispatchMode = dispatchModeOverride;
    else if (runMode == "single")
        currentDispatchMode = singleTrialDispatchMode;
    else if (runMode == "list")
        currentDispatchMode = trialListDispatchMode;
    else
    {
        qDebug() << "ERROR: PlotViewer::dispatch_Impl cannot read trial run info XML.";
        return;
    }
    int action = currentDispatchMode;
    QMutableMapIterator<QSvgWidget*, bool> plotSourceModified_it(plotSourceModified);
    while (plotSourceModified_it.hasNext())
    {
        plotSourceModified_it.next();
        if (plotSourceModified_it.value())
        {
            plotSourceModified_it.setValue(false);
            QSvgWidget* svg = plotSourceModified_it.key();
            switch(action)
            {
            case New:
            {
                QSvgWidget* new_svg = snapshot(svg);
                trialRunInfoMap[new_svg].append(info);
                break;
            }
            case Append: // not implemented for the moment, take it as overwrite
            case Overwrite:
            {
                plotIsUpToDate[svg] = false;
                trialRunInfoMap[svg].append(info);
                break;
            }
            default:
            {
                qDebug() << "PlotViewer::dispatch_Impl computed action was unrecognised";
                return;
            }
            }
        }
    }
    updateActivePlots();
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
       emit sendDrawCmd(plotName.value(currentSvgWidget()));
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
        if (plotName.values().contains(name))
            name.append("_copy");

        addPlot(name);
        setPlotActive(false);
        currentSvgWidget()->load(fileName);
        currentTabChanged(plotPanel->currentIndex()); // refresh action enable properties
    }
}

void PlotViewer::loadByteArray(QString name, QByteArray _byteArray)
{
    QSvgWidget* svg = plotName.key(name);
    if (svg)
    {
//        if(byteArray[svg]) delete(byeArray[svg]);
        plotIsUpToDate[svg]=true;
        byteArray[svg] = _byteArray;
        svg->load(byteArray.value(svg));
        plotPanel->setCurrentWidget(svg);
        titleLabel->setText(name);
    }
}

void PlotViewer::save()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save SVG File"),
                                                    QString("%1/%2").arg(easyNetHome)
                                                    .arg(plotName.value(currentSvgWidget())),
                                                    tr("SVG Files (*.svg)"));
    if (!fileName.isEmpty())
    {
          QFile file(fileName);
          file.open(QIODevice::WriteOnly);
          file.write(byteArray.value(currentSvgWidget()));
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

void PlotViewer::addPlot(QString name, QString sourceDataframe)
{
//    QSvgWidget *svg = new QSvgWidget;
//    plotName[svg] = name;
    if (!sourceDataframe.isEmpty() && !plotDataframeMap.contains(sourceDataframe))
        dataframeFilter->addName(sourceDataframe);

    if (!sourceDataframe.isEmpty())
        plotDataframeMap[sourceDataframe].insert(name);

    newSvg(name);

//    plotPanel->addTab(svg, QString("Plot %1").arg(++progressiveTabIdx));
//    setPlotActive(true, svg);
//    plotIsUpToDate[svg] = false;
//    plotPanel->setCurrentWidget(svg);
//    currentTabChanged(plotPanel->currentIndex());
//    return svg;
}

void PlotViewer::updateActivePlots()
{
    if(plotPanel->currentIndex()>-1 &&
            plotIsActive[currentSvgWidget()] &&
            !plotIsUpToDate[currentSvgWidget()])
    {
        if(visibleRegion().isEmpty())
        {
            pend=true;
        }
        else
        {
            emit sendDrawCmd(plotName.value(currentSvgWidget()));
        }
    }
}

void PlotViewer::updateAllActivePlots()
{
    QMapIterator<QSvgWidget*, bool> plotIsActiveIt(plotIsActive);
    while (plotIsActiveIt.hasNext()) {
        plotIsActiveIt.next();
        if (plotIsActiveIt.value())
            plotIsUpToDate[plotIsActiveIt.key()]=false;
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


void PlotViewer::setPlotActive(bool isActive, QSvgWidget *svg)
{
    svg = svg ? svg : currentSvgWidget();
    plotIsActive[svg] = isActive;
    plotPanel->setTabIcon(plotPanel->indexOf(svg),
                          isActive ? QIcon(":/images/icon-record.png") :
                                     QIcon(":/images/snapshot-icon.png"));
}

QSvgWidget *PlotViewer::newSvg(QString name)
{
    QSvgWidget* svg = new QSvgWidget;
    plotName[svg] = name;
//    plotPanel->addTab(svg, QString("Plot %1").arg(++progressiveTabIdx));
//    setPlotActive(true, svg);
    plotIsUpToDate[svg] = false;
    plotSourceModified[svg] = false;
//    dispatchModeMap[svg] = Overwrite;
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

    setPlotActive(false, svg);
}

void PlotViewer::renamePlot()
{
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename plot",
                                            "New name:                                    ",
                                            QLineEdit::Normal,
                                            plotName.value(currentSvgWidget()), &ok);
    if (ok && !newName.isEmpty())
    {
        plotName[currentSvgWidget()] = newName;
        titleLabel->setText(newName);
    }
}

void PlotViewer::deletePlot()
{
    if (!plotIsActive.value(currentSvgWidget()))
    {
        plotName.remove(currentSvgWidget());
        plotIsActive.remove(currentSvgWidget());
        byteArray.remove(currentSvgWidget());
        delete currentSvgWidget();
    }
}

void PlotViewer::makeSnapshot(QString name)
{
    QSvgWidget* svg = plotName.key(name);
    freeze(svg);
    plotName[svg] = QString("%1_SNAPSHOT").arg(name);
    if (svg == static_cast<QSvgWidget*>(plotPanel->currentWidget()))
        currentTabChanged(plotPanel->currentIndex());
}

void PlotViewer::snapshot()
{
    freeze();
    QSvgWidget* currentSVG = static_cast<QSvgWidget*>(plotPanel->currentWidget());
    QString currentName = plotName.value(currentSVG);
    plotName[currentSVG] = QString("%1_SNAPSHOT").arg(currentName);
    addPlot(currentName);
    plotIsUpToDate[currentSVG]=true;
}

QSvgWidget* PlotViewer::snapshot(QSvgWidget* svg)
{
    QString name = plotName.value(svg);
    freeze(svg);
    plotName[svg] = QString("%1_SNAPSHOT").arg(name);
    return newSvg(name);
}

void PlotViewer::currentTabChanged(int index)
{
    QSvgWidget* svg = static_cast<QSvgWidget*>(plotPanel->widget(index));
    if (plotIsActive.value(svg))
    {
        emit setPlot(plotName.value(svg));
        updateActivePlots();
//        if(!plotIsUpToDate[svg]&&!visibleRegion().isEmpty()) emit sendDrawCmd(plotName.value(svg));
    }
    settingsAct->setEnabled(plotIsActive.value(svg));
    refreshAct->setEnabled(plotIsActive.value(svg));
    snapshotAct->setEnabled(plotIsActive.value(svg));
    renameAct->setEnabled(!plotIsActive.value(svg));
    deleteAct->setEnabled(!plotIsActive.value(svg));
    titleLabel->setText(plotName.value(svg));
}
