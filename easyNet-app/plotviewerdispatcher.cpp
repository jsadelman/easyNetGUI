#include "plotviewerdispatcher.h"
#include "plotviewer.h"
#include "enumclasses.h"
#include "sessionmanager.h"
#include "ui_dataviewer.h"

#include <QAction>
#include <QToolBar>
#include <QSvgWidget>
#include <QDomDocument>

PlotViewerDispatcher::PlotViewerDispatcher(PlotViewer *host)
    :DataViewerDispatcher(host), host(host)
{
    if (!host)
    {
        eNerror << "invalid host PlotViewer, host is" << host;
    }
    setSingleTrialMode(Dispatch_New);
    setTrialListMode(Dispatch_New);
//    host->ui->dispatchToolBar->removeAction(host->ui->setDispatchModeOverrideActs.at(Dispatch_Append));
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
    foreach (QString plot, affectedPlots(trialRunInfo.results))
    {
        if (!host->plotByteArray.contains(plot))
        {
            currentDispatchAction = Dispatch_Overwrite;
        }
//        else if (!dispatchModeAuto && dispatchModeOverride > -1)
//        {
//            currentDispatchAction = dispatchModeOverride;
//        }
        else if (!SessionManager::instance()->dataframeDependencies(plot).intersect(SessionManager::instance()->enabledObservers().toSet()).isEmpty())
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
            /*QString newPlotName =*/ host->snapshot(plot);
//            copyTrialRunInfo(rplot, newPlotName);
        }
    }
    previousDispatchMode = currentDispatchMode;
}

void PlotViewerDispatcher::dispatch(QSharedPointer<QDomDocument> info)
{
    foreach(QString plot, affectedPlots(TrialRunInfo(info).results))
    {
        SessionManager::instance()->setTrialRunInfo(plot, info);
        updateHistory(plot);
//        host->plotIsUpToDate[plot] = false;
    }
//    QMutableMapIterator<QString, bool> plotSourceModified_it(host->plotSourceModified);
//    while (plotSourceModified_it.hasNext())
//    {
//        plotSourceModified_it.next();
//        if (plotSourceModified_it.value())
//        {
//            plotSourceModified_it.setValue(false);
//            QString plot = plotSourceModified_it.key();
//            host->plotIsUpToDate[plot] = false;
////            host->svgTrialRunInfo[svg] = info;
//        }
//    }
//    host->updateActivePlots();
    if (infoIsVisible)
        showInfo(true);
}


QStringList PlotViewerDispatcher::affectedPlots(QString results)
{
    QStringList plots;
    foreach (QString plot, host->items())
    {
        QSet<QString> dependencies = SessionManager::instance()->dataframeDependencies(plot);
        if (    dependencies.contains(results) ||
                (!SessionManager::instance()->suspendingObservers()) &&
                (!dependencies.intersect(SessionManager::instance()->enabledObservers().toSet()).isEmpty())
           )
                plots.append(plot);
    }
    return plots;


//    foreach (QString plot, host->plotDependencies.keys())
//    {
//        // if either results or any enabled (and not suspended) observer are in the dependency list of this plot
//        if (   host->plotDependencies.values(plot).contains(results) ||
//              (!SessionManager::instance()->suspendingObservers()) &&
//              (!host->plotDependencies.values(plot).toSet().intersect(SessionManager::instance()->enabledObservers().toSet()).isEmpty()) )
//            plots.append(plot);
//    }
//    return plots;
}

