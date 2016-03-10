#include "plotviewerdispatcher.h"
#include "plotviewer.h"
#include "enumclasses.h"
#include "sessionmanager.h"
#include "ui_dataviewer.h"

#include <QAction>
#include <QToolBar>
#include <QSvgWidget>

PlotViewerDispatcher::PlotViewerDispatcher(PlotViewer *host)
    :DataViewerDispatcher(host), host(host)
{
    if (!host)
    {
        eNerror << "invalid host PlotViewer, host is" << host;
    }
    setSingleTrialMode(Dispatch_New);
    setTrialListMode(Dispatch_New);
    host->ui->dispatchToolBar->removeAction(host->ui->setDispatchModeOverrideActs.at(Dispatch_Append));
}

PlotViewerDispatcher::~PlotViewerDispatcher()
{

}

void PlotViewerDispatcher::preDispatch(QSharedPointer<QDomDocument> info)
{
    TrialRunInfo trialRunInfo(info);
    int currentDispatchMode = dispatchDefaultMode.value(trialRunInfo.runMode, -1);
    if (currentDispatchMode < 0 || currentDispatchMode >= MAX_DISPATCH_MODE)
    {
        eNerror << QString("cannot retrieve a valid runMode from trial run info; runMode is %1")
                   .arg(trialRunInfo.runMode);
        return;
    }
    foreach (QString rplot, SessionManager::instance()->affectedPlots(trialRunInfo.results))
    {
//        QSvgWidget* svg = qobject_cast<QSvgWidget*>(host->ui->view(rplot));
//        currentDispatchAction;
        if (!host->plotByteArray.contains(rplot))
        {
            currentDispatchAction = Dispatch_Overwrite;
        }
        else if (!dispatchModeAuto && dispatchModeOverride > -1)
        {
            currentDispatchAction = dispatchModeOverride;
        }
        else if (SessionManager::instance()->isAnyTrialPlot(rplot))
        {
            currentDispatchAction = Dispatch_Overwrite;
        }
        else if (previousDispatchMode < 0)
        {
            currentDispatchAction = currentDispatchMode;
        }
        else
        {
            currentDispatchAction = dispatchModeFST.value(qMakePair(previousDispatchMode, currentDispatchMode));
        }
        if (currentDispatchAction == Dispatch_New)
        {
            /*QString newPlotName =*/ host->cloneRPlot(rplot);
//            copyTrialRunInfo(rplot, newPlotName);
        }
    }
    previousDispatchMode = currentDispatchMode;
}

void PlotViewerDispatcher::dispatch(QSharedPointer<QDomDocument> info)
{
    foreach(QString plot, SessionManager::instance()->affectedPlots(TrialRunInfo(info).results))
    {
        updateHistory(plot, info);
//        setTrialRunInfo(plot, info);
    }
    QMutableMapIterator<QString, bool> plotSourceModified_it(host->plotSourceModified);
    while (plotSourceModified_it.hasNext())
    {
        plotSourceModified_it.next();
        if (plotSourceModified_it.value())
        {
            plotSourceModified_it.setValue(false);
            QString plot = plotSourceModified_it.key();
            host->plotIsUpToDate[plot] = false;
//            host->svgTrialRunInfo[svg] = info;
        }
    }
    host->updateActivePlots();
    if (infoIsVisible)
        showInfo(true);
//    host->refreshInfo();
}

