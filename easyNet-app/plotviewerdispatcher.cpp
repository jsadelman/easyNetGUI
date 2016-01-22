#include "plotviewerdispatcher.h"
#include "plotviewer.h"
#include "enumclasses.h"
#include "sessionmanager.h"
#include "ui_dataviewer.h"

#include <QAction>
#include <QToolBar>

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
        QSvgWidget* svg = host->plotSvg[rplot];
        int dispatchAction;
        if (!host->svgByteArray.contains(svg))
        {
            dispatchAction = Dispatch_Overwrite;
        }
        else if (!dispatchModeAuto && dispatchModeOverride > -1 && svg == host->currentSvgWidget())
        {
            dispatchAction = dispatchModeOverride;
        }
        else if (SessionManager::instance()->isAnyTrialPlot(rplot))
        {
            dispatchAction = Dispatch_Overwrite;
        }
        else
        {
            dispatchAction = dispatchModeFST.value(qMakePair(previousDispatchMode, currentDispatchMode));
        }
        if (dispatchAction == Dispatch_New)
        {
            host->cloneRPlot(host->plotSvg.key(svg));
        }
    }
    previousDispatchMode = currentDispatchMode;
}

void PlotViewerDispatcher::dispatch(QSharedPointer<QDomDocument> info)
{
    QMutableMapIterator<QSvgWidget*, bool> svgSourceModified_it(host->svgSourceModified);
    while (svgSourceModified_it.hasNext())
    {
        svgSourceModified_it.next();
        if (svgSourceModified_it.value())
        {
            svgSourceModified_it.setValue(false);
            QSvgWidget* svg = svgSourceModified_it.key();
            host->svgIsUpToDate[svg] = false;
            host->svgTrialRunInfo[svg] = info;
        }
    }
    host->updateActivePlots();
//    host->refreshInfo();
}

