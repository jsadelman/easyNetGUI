#include "plotviewerdispatcher.h"
#include "plotviewer.h"
#include "enumclasses.h"
#include "sessionmanager.h"
#include "lazynutjob.h"
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
        if (host->plotByteArray.contains(plot) && snapshotActive())
            host->snapshot(plot);

//        if (!host->plotByteArray.contains(plot))
//        {
//            currentDispatchAction = Dispatch_Overwrite;
//        }
//        else if (!dispatchModeAuto && dispatchModeOverride > -1)
//        {
//            currentDispatchAction = dispatchModeOverride;
//        }
//        else if (!SessionManager::instance()->dataframeDependencies(plot).intersect(SessionManager::instance()->enabledObservers().toSet()).isEmpty())
//        {
//            currentDispatchAction = Dispatch_Overwrite;
//        }
//        else if (previousDispatchMode < 0)
//        {
//            currentDispatchAction = currentDispatchMode;
//        }
//        else
//        {
//            currentDispatchAction = dispatchModeFST.value(qMakePair(previousDispatchMode, currentDispatchMode));
//        }
        if (host->plotByteArray.contains(plot) && copyDfActive())
        {
            foreach(QString df, SessionManager::instance()->dataframeDependencies(plot))
            {
                if (df == trialRunInfo.results ||
                    (!SessionManager::instance()->suspendingObservers()) &&
                     SessionManager::instance()->enabledObservers().contains(df))
                {
                    if (!SessionManager::instance()->isCopyRequested(df))
                    {
                        SessionManager::instance()->setCopyRequested(df);
                        QString copyDf = SessionManager::instance()->makeValidObjectName(QString("%1.Copy.1").arg(df));
                        LazyNutJob *job = new LazyNutJob;
                        job->logMode |= ECHO_INTERPRETER;
                        job->cmdList = QStringList();
                        QList<LazyNutJob*> jobs = QList<LazyNutJob*>() << job;
                        job->cmdList << QString("%1 copy %2").arg(df).arg(copyDf);
                        QMap<QString, QVariant> jobData;
                        jobData.insert("original", df);
                        jobData.insert("name", copyDf);
                        jobData.insert("isBackup", true);
                        jobs << SessionManager::instance()->recentlyCreatedJob();
                        jobs.last()->data = jobData;
                        jobs.last()->appendEndOfJobReceiver(host, SLOT(requestAddDataframe()));
                        jobs.last()->appendEndOfJobReceiver(SessionManager::instance(), SLOT(clearCopyRequested()));
                        SessionManager::instance()->submitJobs(jobs);
                        SessionManager::instance()->copyTrialRunInfo(plot, copyDf);
                    }
                }
             }
        }
    }
    previousDispatchMode = currentDispatchMode;
}

void PlotViewerDispatcher::dispatch(QSharedPointer<QDomDocument> info)
{
    if (!SessionManager::instance()->suspendingObservers())
        foreach (QString observer, SessionManager::instance()->enabledObservers())
            SessionManager::instance()->setTrialRunInfo(observer, info);

    foreach(QString plot, affectedPlots(TrialRunInfo(info).results))
    {
//        SessionManager::instance()->setTrialRunInfo(plot, info);
        updateHistory(plot);
//        host->plotIsUpToDate[plot] = false;
    }
    if (infoIsVisible)
        showInfo(true);
}

QDomDocument *PlotViewerDispatcher::makePreferencesDomDoc()
{
    QDomDocument *domDoc = new QDomDocument;
    QDomElement eNelements = domDoc->createElement("eNelements");
    domDoc->appendChild(eNelements);


    QDomElement snapshots = domDoc->createElement("map");
    snapshots.setAttribute("label", "Snapshots");
    eNelements.appendChild(snapshots);

    QDomElement snapshotsType = domDoc->createElement("string");
    snapshotsType.setAttribute("label", "type");
    snapshotsType.setAttribute("value", "factor");
    snapshots.appendChild(snapshotsType);

    QDomElement snapshotsLevels = domDoc->createElement("list");
    snapshotsLevels.setAttribute("label", "levels");
    QDomElement snapshotsLevelsYes = domDoc->createElement("string");
    snapshotsLevelsYes.setAttribute("value", "yes");
    snapshotsLevels.appendChild(snapshotsLevelsYes);
    QDomElement snapshotsLevelsNo = domDoc->createElement("string");
    snapshotsLevelsNo.setAttribute("value", "no");
    snapshotsLevels.appendChild(snapshotsLevelsNo);
    snapshots.appendChild(snapshotsLevels);

    QDomElement snapshotsValue = domDoc->createElement("string");
    snapshotsValue.setAttribute("label", "value");
    snapshotsValue.setAttribute("value", snapshotActive() ? "yes" : "no");
    snapshots.appendChild(snapshotsValue);

    QDomElement snapshotsDefault = domDoc->createElement("string");
    snapshotsDefault.setAttribute("label", "default");
    snapshotsDefault.setAttribute("value", snapshotActive() ? "yes" : "no");
    snapshots.appendChild(snapshotsDefault);

    QDomElement snapshotsComment = domDoc->createElement("string");
    snapshotsComment.setAttribute("label", "comment");
    snapshotsComment.setAttribute("value", QString("Take a snapshot (SVG format) of plots before they get modified by a trial run"));
    snapshots.appendChild(snapshotsComment);

    QDomElement snapshotsChoice = domDoc->createElement("string");
    snapshotsChoice.setAttribute("label", "choice");
    snapshotsChoice.setAttribute("value", "single");
    snapshots.appendChild(snapshotsChoice);


    QDomElement copyDf = domDoc->createElement("map");
    copyDf.setAttribute("label", "Copy source dataframes");
    eNelements.appendChild(copyDf);

    QDomElement copyDfType = domDoc->createElement("string");
    copyDfType.setAttribute("label", "type");
    copyDfType.setAttribute("value", "factor");
    copyDf.appendChild(copyDfType);

    QDomElement copyDfLevels = domDoc->createElement("list");
    copyDfLevels.setAttribute("label", "levels");
    QDomElement copyDfLevelsYes = domDoc->createElement("string");
    copyDfLevelsYes.setAttribute("value", "yes");
    copyDfLevels.appendChild(copyDfLevelsYes);
    QDomElement copyDfLevelsNo = domDoc->createElement("string");
    copyDfLevelsNo.setAttribute("value", "no");
    copyDfLevels.appendChild(copyDfLevelsNo);
    copyDf.appendChild(copyDfLevels);

    QDomElement copyDfValue = domDoc->createElement("string");
    copyDfValue.setAttribute("label", "value");
    copyDfValue.setAttribute("value", copyDfActive() ? "yes" : "no");
    copyDf.appendChild(copyDfValue);

    QDomElement copyDfDefault = domDoc->createElement("string");
    copyDfDefault.setAttribute("label", "default");
    copyDfDefault.setAttribute("value", copyDfActive() ? "yes" : "no");
    copyDf.appendChild(copyDfDefault);

    QDomElement copyDfComment = domDoc->createElement("string");
    copyDfComment.setAttribute("label", "comment");
    copyDfComment.setAttribute("value", QString("Copy plot source dataframes before they get modified by a trial run.\n"
                                                "WARNING: can generate very large dataframes."));
    copyDf.appendChild(copyDfComment);

    QDomElement copyDfChoice = domDoc->createElement("string");
    copyDfChoice.setAttribute("label", "choice");
    copyDfChoice.setAttribute("value", "single");
    copyDf.appendChild(copyDfChoice);


    //    qDebug() << preferencesXML->toString();

    return domDoc;
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
}

