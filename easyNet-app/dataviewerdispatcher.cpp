#include "dataviewerdispatcher.h"
#include "enumclasses.h"
#include "xmlaccessor.h"
#include "dataviewer.h"

DataViewerDispatcher::DataViewerDispatcher(DataViewer *host)
    : QObject(host), hostDataViewer(host), dispatchModeOverride(-1), dispatchModeAuto(true), previousDispatchMode(-1)
{
    if (!host)
    {
        eNerror << "invalid host DataViewer, host is" << host;
    }
    host->setDispatcher(this);


    dispatchDefaultMode.insert("single", -1);
    dispatchDefaultMode.insert("list", -1);

    dispatchModeFST.insert(qMakePair(Dispatch_New,Dispatch_New), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_New,Dispatch_Overwrite), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_New,Dispatch_Append), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_Overwrite,Dispatch_New), Dispatch_Overwrite);
    dispatchModeFST.insert(qMakePair(Dispatch_Overwrite,Dispatch_Overwrite), Dispatch_Overwrite);
    dispatchModeFST.insert(qMakePair(Dispatch_Overwrite,Dispatch_Append), Dispatch_Overwrite);
    dispatchModeFST.insert(qMakePair(Dispatch_Append,Dispatch_New), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_Append,Dispatch_Overwrite), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_Append,Dispatch_Append), Dispatch_Append);
}

DataViewerDispatcher::~DataViewerDispatcher()
{
}

void DataViewerDispatcher::dispatch(QSharedPointer<QDomDocument> info)
{
    TrialRunInfo trialRunInfo(info);
    setTrialRunInfo(trialRunInfo.results, info);
//    qDebug() << info->toString();
}

void DataViewerDispatcher::setTrialRunInfo(QString item, QSharedPointer<QDomDocument> info)
{
    trialRunInfoMap.insert(item, info);
}

void DataViewerDispatcher::copyTrialRunInfo(QString fromItem, QString toItem)
{
    if (trialRunInfoMap.contains(fromItem))
        trialRunInfoMap.insert(toItem, trialRunInfoMap.value(fromItem));
}

QString DataViewerDispatcher::getTrial(QString item)
{
    if (trialRunInfoMap.contains(item))
        return TrialRunInfo(trialRunInfoMap.value(item)).trial;
    return QString();
}

QString DataViewerDispatcher::getRunMode(QString item)
{
    if (trialRunInfoMap.contains(item))
        return TrialRunInfo(trialRunInfoMap.value(item)).runMode;
    return QString();
}

QString DataViewerDispatcher::getResults(QString item)
{
    if (trialRunInfoMap.contains(item))
        return TrialRunInfo(trialRunInfoMap.value(item)).results;
    return QString();
}


DataViewerDispatcher::TrialRunInfo::TrialRunInfo(QSharedPointer<QDomDocument> info)
{
    QDomElement rootElement = info->documentElement();
    QDomElement resultsElement = XMLAccessor::childElement(rootElement, "Results");
    results = XMLAccessor::value(resultsElement);
    QDomElement trialElement = XMLAccessor::childElement(rootElement, "Trial");
    trial = XMLAccessor::value(trialElement);
    QDomElement runModeElement = XMLAccessor::childElement(rootElement, "Run mode");
    runMode = XMLAccessor::value(runModeElement);
}
