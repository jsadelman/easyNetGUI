#include "dataframeviewerdispatcher.h"
#include "enumclasses.h"
#include "xmlaccessor.h"
#include "sessionmanager.h"
#include "lazynutjob.h"
#include "dataframeviewer.h"
#include "objectcache.h"

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
    int dispatchAction;
    if (!inHistory(trialRunInfo.results))
    {
        dispatchAction = Dispatch_Overwrite;
    }
    else if (!dispatchModeAuto && dispatchModeOverride > -1)
    {
        dispatchAction = dispatchModeOverride;
    }
    else if (previousDispatchMode < 0)
    {
        dispatchAction = currentDispatchMode;
    }
    else
    {
        dispatchAction = dispatchModeFST.value(qMakePair(previousDispatchMode, currentDispatchMode));
    }
    previousDispatchMode = currentDispatchMode;
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList = QStringList();
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>() << job;
    switch(dispatchAction)
    {
    case Dispatch_New:
    {
        QDomDocument* description = SessionManager::instance()->descriptionCache->getDomDoc(trialRunInfo.results);
        QDomElement rootElement = description->documentElement();
        QDomElement prettyNameElement = XMLAccessor::childElement(rootElement, "pretty name");
        QString prettyName = XMLAccessor::value(prettyNameElement);
        QString backupDf = SessionManager::instance()->makeValidObjectName(prettyName);
        job->cmdList << QString("%1 copy %2").arg(trialRunInfo.results).arg(backupDf);
        job->cmdList << QString("%1 clear").arg(trialRunInfo.results);
        QMap<QString, QVariant> jobData;
        jobData.insert("name", backupDf);
        jobData.insert("setCurrent", false);
        jobData.insert("isBackup", true);
        QVariant infoVariant;
        infoVariant.setValue(trialRunInfoMap.value(trialRunInfo.results));
        jobData.insert("trialRunInfo", infoVariant);
        jobs << SessionManager::instance()->recentlyCreatedJob();
        jobs.last()->data = jobData;
        jobs.last()->appendEndOfJobReceiver(host, SLOT(addItem()));
//        copyTrialRunInfo(trialRunInfo.results, backupDf);
        host->setPrettyHeadersForTrial(trialRunInfo.trial, backupDf);
        break;
    }
    case Dispatch_Overwrite:
    {
        job->cmdList << QString("%1 clear").arg(trialRunInfo.results);
        if (!host->contains(trialRunInfo.results))
        {
            host->addItem(trialRunInfo.results, true, false, info);
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
    updateHistory(results, info);
    setTrialRunInfo(results, info);
    if (infoAct->isChecked())
        showInfo(true);
}


