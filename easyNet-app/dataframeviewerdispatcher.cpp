#include "dataframeviewerdispatcher.h"
#include "enumclasses.h"
#include "xmlaccessor.h"
#include "sessionmanager.h"
#include "lazynutjob.h"
#include "dataframeviewer.h"
#include "objectcache.h"
#include "objectcachefilter.h"

#include <QSharedPointer>
#include <QDomDocument>
#include <QAction>

Q_DECLARE_METATYPE(QSharedPointer<QDomDocument> )


DataframeViewerDispatcher::DataframeViewerDispatcher(DataframeViewer *host)
    :DataViewerDispatcher(host), host(host)
{
    if (!host)
    {
        eNerror << "invalid host DataframeViewer, host is" << host;
    }
    setSingleTrialMode(Dispatch_Append);
    setTrialListMode(Dispatch_New);
}

DataframeViewerDispatcher::~DataframeViewerDispatcher()
{
}

void DataframeViewerDispatcher::preDispatch(QSharedPointer<QDomDocument> info)
{
    TrialRunInfo trialRunInfo(info);
    int currentDispatchMode = dispatchDefaultMode.value(trialRunInfo.runMode, -1);
    if (currentDispatchMode < 0 || currentDispatchMode >= MAX_DISPATCH_MODE)
    {
        eNerror << QString("cannot retrieve a valid runMode from trial run info; runMode is %1")
                   .arg(trialRunInfo.runMode);
        return;
    }
//    int currentDispatchAction;
    if (!inHistory(trialRunInfo.results))
    {
        currentDispatchAction = Dispatch_Overwrite;
    }
    else if (!dispatchModeAuto && dispatchModeOverride > -1)
    {
        currentDispatchAction = dispatchModeOverride;
    }
    else if (previousDispatchMode < 0)
    {
        currentDispatchAction = currentDispatchMode;
    }
    else
    {
        currentDispatchAction = dispatchModeFST.value(qMakePair(previousDispatchMode, currentDispatchMode));
    }
    previousDispatchMode = currentDispatchMode;
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList = QStringList();
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>() << job;
    switch(currentDispatchAction)
    {
    case Dispatch_New:
    {
//        host->dataframeDescriptionFilter->rowCount();

//        QDomDocument* description = SessionManager::instance()->descriptionCache->getDomDoc(trialRunInfo.results);
//        QDomElement rootElement = description->documentElement();
//        QDomElement prettyNameElement = XMLAccessor::childElement(rootElement, "pretty name");
//        QString prettyName = XMLAccessor::value(prettyNameElement);
        QString backupDf = SessionManager::instance()->makeValidObjectName(trialRunInfo.results);
        job->cmdList << QString("%1 copy %2").arg(trialRunInfo.results).arg(backupDf);
        job->cmdList << QString("%1 clear").arg(trialRunInfo.results);
        QMap<QString, QVariant> jobData;
        jobData.insert("name", backupDf);
        jobData.insert("setCurrent", false);
        jobData.insert("isBackup", true);
        jobData.insert("trial", trial(trialRunInfo.results)); // the current trial, not the future one, even though for df they are probably the same
//        QVariant infoVariant = infoVariantList(trialRunInfo.results);
//        infoVariant.setValue(trialRunInfoMap.value(trialRunInfo.results));
//        jobData.insert("trialRunInfo", infoVariant);
        jobs << SessionManager::instance()->recentlyCreatedJob();
        jobs.last()->data = jobData;
        jobs.last()->appendEndOfJobReceiver(host, SLOT(addItem()));
        SessionManager::instance()->copyTrialRunInfo(trialRunInfo.results, backupDf);
//        copyTrialRunInfo(trialRunInfo.results, backupDf);
        host->setPrettyHeadersForTrial(trialRunInfo.trial, backupDf);
        break;
    }
    case Dispatch_Overwrite:
    {
        job->cmdList << QString("%1 clear").arg(trialRunInfo.results);
        if (!host->contains(trialRunInfo.results))
        {
            SessionManager::instance()->setTrialRunInfo(trialRunInfo.results, info);
            host->addItem(trialRunInfo.results, false);
            host->setPrettyHeadersForTrial(trialRunInfo.trial, trialRunInfo.results);
        }
        break;
    }
    case Dispatch_Append:
    {
        // don't send results clear
        break;
    }
    default:
    {
        eNerror << "the computed dispatch action was not recognised";
    }
    }
    if (!job->cmdList.isEmpty())
    {
        SessionManager::instance()->submitJobs(jobs);
    }
    else
    {
        foreach(LazyNutJob *j, jobs)
            delete j;
    }
}

void DataframeViewerDispatcher::dispatch(QSharedPointer<QDomDocument> info)
{
    QString results = TrialRunInfo(info).results;
//    updateHistory(results, info);
    switch(currentDispatchAction)
    {
    case Dispatch_New:
        ;
    case Dispatch_Overwrite:
        SessionManager::instance()->setTrialRunInfo(results, info);
        break;
    case Dispatch_Append:
        SessionManager::instance()->appendTrialRunInfo(results, info);
        break;
    default:
        eNerror << "the computed dispatch action was not recognised";
    }
    if (infoIsVisible)
        showInfo(true);
}


