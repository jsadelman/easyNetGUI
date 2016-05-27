#include "dataframeviewerdispatcher.h"
#include "enumclasses.h"
#include "xmlaccessor.h"
#include "sessionmanager.h"
#include "lazynutjob.h"
#include "dataframeviewer.h"
#include "objectcache.h"
#include "objectcachefilter.h"
#include "historytreemodel.h"

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
//    else if (!dispatchModeAuto && dispatchModeOverride > -1)
//    {
//        currentDispatchAction = dispatchModeOverride;
//    }
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
    job->cmdList = QStringList();
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>() << job;
//    qDebug () << Q_FUNC_INFO << dispatchModeText.value(currentDispatchAction);
    switch(currentDispatchAction)
    {
    case Dispatch_New:
    {
        if (!SessionManager::instance()->isCopyRequested(trialRunInfo.results))
        {
            SessionManager::instance()->setCopyRequested(trialRunInfo.results);
            QString backupDf = SessionManager::instance()->makeValidObjectName(QString("%1.Copy.1").arg(trialRunInfo.results));
            job->cmdList << QString("%1 copy %2").arg(trialRunInfo.results).arg(backupDf);
            job->cmdList << QString("%1 add_hint show 0").arg(backupDf);
            job->cmdList << QString("%1 clear").arg(trialRunInfo.results);
            QMap<QString, QVariant> jobData;
            jobData.insert("original", trialRunInfo.results);
            jobData.insert("name", backupDf);
            jobData.insert("isBackup", true);
            jobs << SessionManager::instance()->updateObjectCacheJobs();
            jobs.last()->data = jobData;
            jobs.last()->appendEndOfJobReceiver(host, SLOT(addItem()));
            jobs.last()->appendEndOfJobReceiver(SessionManager::instance(), SLOT(clearCopyRequested()));
            SessionManager::instance()->copyTrialRunInfo(trialRunInfo.results, backupDf);
        }
        break;
    }
    case Dispatch_Overwrite:
    {
        job->cmdList << QString("%1 clear").arg(trialRunInfo.results);
        if (!host->contains(trialRunInfo.results))
        {
            SessionManager::instance()->setTrialRunInfo(trialRunInfo.results, info);
            host->addItem(trialRunInfo.results, false);
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
    if (!historyModel->isInView(results))
        historyModel->setInView(results, true);

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

QDomDocument *DataframeViewerDispatcher::makePreferencesDomDoc()
{
    QDomDocument *domDoc = new QDomDocument;
    QDomElement eNelements = domDoc->createElement("eNelements");
    domDoc->appendChild(eNelements);


    QDomElement singleTrialRun = domDoc->createElement("map");
    singleTrialRun.setAttribute("label", trialRunModeName.value(TrialRunMode_Single));
    eNelements.appendChild(singleTrialRun);

    QDomElement singleTrialRunType = domDoc->createElement("string");
    singleTrialRunType.setAttribute("label", "type");
    singleTrialRunType.setAttribute("value", "factor");
    singleTrialRun.appendChild(singleTrialRunType);

    QDomElement singleTrialRunLevels = domDoc->createElement("list");
    singleTrialRunLevels.setAttribute("label", "levels");
    QDomElement singleTrialRunLevelsNew = domDoc->createElement("string");
    singleTrialRunLevelsNew.setAttribute("value", dispatchModeText.value(Dispatch_New));
    singleTrialRunLevels.appendChild(singleTrialRunLevelsNew);
    QDomElement singleTrialRunLevelsAppend = domDoc->createElement("string");
    singleTrialRunLevelsAppend.setAttribute("value", dispatchModeText.value(Dispatch_Append));
    singleTrialRunLevels.appendChild(singleTrialRunLevelsAppend);
    QDomElement singleTrialRunLevelsOverwrite = domDoc->createElement("string");
    singleTrialRunLevelsOverwrite.setAttribute("value", dispatchModeText.value(Dispatch_Overwrite));
    singleTrialRunLevels.appendChild(singleTrialRunLevelsOverwrite);
    singleTrialRun.appendChild(singleTrialRunLevels);

    QDomElement singleTrialRunValue = domDoc->createElement("string");
    singleTrialRunValue.setAttribute("label", "value");
    singleTrialRunValue.setAttribute("value", dispatchModeText.value(dispatchDefaultMode.value(trialRunModeName.value(TrialRunMode_Single))));
    singleTrialRun.appendChild(singleTrialRunValue);

    QDomElement singleTrialRunDefault = domDoc->createElement("string");
    singleTrialRunDefault.setAttribute("label", "default");
    singleTrialRunDefault.setAttribute("value", dispatchModeText.value(dispatchDefaultMode.value(trialRunModeName.value(TrialRunMode_Single))));
    singleTrialRun.appendChild(singleTrialRunDefault);

    QDomElement singleTrialRunComment = domDoc->createElement("string");
    singleTrialRunComment.setAttribute("label", "comment");
    singleTrialRunComment.setAttribute("value", QString("Destination of results from a single trial run:\n"
                                       "`%1`: send to a new table\n"
                                       "`%2`: append to current table\n"
                                       "`%3`: overwrite current table")
                                       .arg(dispatchModeText.value(Dispatch_New))
                                       .arg(dispatchModeText.value(Dispatch_Append))
                                       .arg(dispatchModeText.value(Dispatch_Overwrite)));
    singleTrialRun.appendChild(singleTrialRunComment);

    QDomElement singleTrialRunChoice = domDoc->createElement("string");
    singleTrialRunChoice.setAttribute("label", "choice");
    singleTrialRunChoice.setAttribute("value", "single");
    singleTrialRun.appendChild(singleTrialRunChoice);


    QDomElement listTrialRun = domDoc->createElement("map");
    listTrialRun.setAttribute("label", trialRunModeName.value(TrialRunMode_List));
    eNelements.appendChild(listTrialRun);

    QDomElement listTrialRunType = domDoc->createElement("string");
    listTrialRunType.setAttribute("label", "type");
    listTrialRunType.setAttribute("value", "factor");
    listTrialRun.appendChild(listTrialRunType);

    QDomElement listTrialRunLevels = domDoc->createElement("list");
    listTrialRunLevels.setAttribute("label", "levels");
    QDomElement listTrialRunLevelsNew = domDoc->createElement("string");
    listTrialRunLevelsNew.setAttribute("value", dispatchModeText.value(Dispatch_New));
    listTrialRunLevels.appendChild(listTrialRunLevelsNew);
    QDomElement listTrialRunLevelsAppend = domDoc->createElement("string");
    listTrialRunLevelsAppend.setAttribute("value", dispatchModeText.value(Dispatch_Append));
    listTrialRunLevels.appendChild(listTrialRunLevelsAppend);
    QDomElement listTrialRunLevelsOverwrite = domDoc->createElement("string");
    listTrialRunLevelsOverwrite.setAttribute("value", dispatchModeText.value(Dispatch_Overwrite));
    listTrialRunLevels.appendChild(listTrialRunLevelsOverwrite);
    listTrialRun.appendChild(listTrialRunLevels);

    QDomElement listTrialRunValue = domDoc->createElement("string");
    listTrialRunValue.setAttribute("label", "value");
    listTrialRunValue.setAttribute("value", dispatchModeText.value(dispatchDefaultMode.value(trialRunModeName.value(TrialRunMode_List))));
    listTrialRun.appendChild(listTrialRunValue);

    QDomElement listTrialRunDefault = domDoc->createElement("string");
    listTrialRunDefault.setAttribute("label", "default");
    listTrialRunDefault.setAttribute("value", dispatchModeText.value(dispatchDefaultMode.value(trialRunModeName.value(TrialRunMode_List))));
    listTrialRun.appendChild(listTrialRunDefault);

    QDomElement listTrialRunComment = domDoc->createElement("string");
    listTrialRunComment.setAttribute("label", "comment");
    listTrialRunComment.setAttribute("value", QString("Destination of results from a list trial run:\n"
                                       "`%1`: send to a new table\n"
                                       "`%2`: append to current table\n"
                                       "`%3`: overwrite current table")
                                       .arg(dispatchModeText.value(Dispatch_New))
                                       .arg(dispatchModeText.value(Dispatch_Append))
                                       .arg(dispatchModeText.value(Dispatch_Overwrite)));
    listTrialRun.appendChild(listTrialRunComment);

    QDomElement listTrialRunChoice = domDoc->createElement("string");
    listTrialRunChoice.setAttribute("label", "choice");
    listTrialRunChoice.setAttribute("value", "single");
    listTrialRun.appendChild(listTrialRunChoice);

    //    qDebug() << preferencesXML->toString();

    return domDoc;
}


