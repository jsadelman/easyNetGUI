#include "dataviewerdispatcher.h"
#include "ui_dataviewer.h"
#include "enumclasses.h"
#include "xmlaccessor.h"
#include "dataviewer.h"
#include "historymodel.h"
#include "historywidget.h"
#include "sessionmanager.h"

#include <QAction>
#include <QToolBar>

DataViewerDispatcher::DataViewerDispatcher(DataViewer *host)
    : QObject(host), hostDataViewer(host), dispatchModeOverride(-1),
      dispatchModeAuto(true), previousDispatchMode(-1), previousItem("")
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

    // history
    historyModel = new CheckListModel(this);
    historyWidget = new HistoryWidget;
    historyWidget->setModel(historyModel);
    historyWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    host->ui->addDockWidget(Qt::LeftDockWidgetArea, historyWidget);
    connect(historyWidget->moveToViewerAct, SIGNAL(triggered()), this, SLOT(moveFromHistoryToViewer()));
    connect(historyWidget->destroyAct, SIGNAL(triggered()), this, SLOT(removeFromHistory()));
    connect(historyWidget, SIGNAL(clicked(QString)), this, SLOT(displayItemFromHistory(QString)));
    historyAct = historyWidget->toggleViewAction();
    historyAct->setIcon(QIcon(":/images/History.png"));
    historyAct->setText("History");
    historyAct->setToolTip("show/hide history");
    connect(historyAct, SIGNAL(triggered(bool)), this, SLOT(setHistoryVisible(bool)));
    setHistoryVisible(false);
    host->ui->dispatchToolBar->addAction(historyAct);
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

QString DataViewerDispatcher::getTrial(QString name)
{
    if (trialRunInfoMap.contains(name))
        return TrialRunInfo(trialRunInfoMap.value(name)).trial;
    return QString();
}

QString DataViewerDispatcher::getRunMode(QString name)
{
    if (trialRunInfoMap.contains(name))
        return TrialRunInfo(trialRunInfoMap.value(name)).runMode;
    return QString();
}

QString DataViewerDispatcher::getResults(QString name)
{
    if (trialRunInfoMap.contains(name))
        return TrialRunInfo(trialRunInfoMap.value(name)).results;
    return QString();
}

void DataViewerDispatcher::addToHistory(QString name)
{
    historyModel->create(name);
}

void DataViewerDispatcher::moveFromHistoryToViewer()
{
    foreach(QString name, historyModel->selectedItems())
    {
        historyModel->destroy(name);
        if (!hostDataViewer->ui->contains(name))
        {
            hostDataViewer->ui->addView(name, hostDataViewer->makeView());
            if (!hostDataViewer->isLazy())
                hostDataViewer->addNameToFilter(name);
        }
        hostDataViewer->ui->setCurrentItem(name);
    }
}

void DataViewerDispatcher::moveFromViewerToHistory(QString name)
{
    historyModel->create(name);
    hostDataViewer->ui->takeView(name);
    if (!hostDataViewer->isLazy())
        hostDataViewer->removeNameFromFilter(name);
}

bool DataViewerDispatcher::inHistory(QString name)
{
    return historyModel->contains(name);
}

void DataViewerDispatcher::removeFromHistory()
{
    foreach(QString item, historyModel->selectedItems())
    {
        SessionManager::instance()->destroyObject(item);
        historyModel->destroy(item);
    }
}

void DataViewerDispatcher::displayItemFromHistory(QString name)
{
    if (name.isEmpty())
        return;
    if (hostDataViewer->ui->contains(name))
    {
        if (hostDataViewer->ui->currentItemName() != name)
        {
            hostDataViewer->ui->setCurrentItem(name);
        }
        return;
    }
    removePreviousItem();
    hostDataViewer->ui->addView(name, hostDataViewer->ui->view(name));
    if (!hostDataViewer->isLazy())
        hostDataViewer->addNameToFilter(name);
    hostDataViewer->ui->setCurrentItem(name);
    previousItem = name;
}

void DataViewerDispatcher::setHistoryVisible(bool visible)
{
    historyWidget->setVisible(visible);
    if (!visible)
        removePreviousItem();

}

void DataViewerDispatcher::removePreviousItem()
{
    if (historyModel->contains(previousItem))
    {
        hostDataViewer->ui->takeView(previousItem);
        if (!hostDataViewer->isLazy())
            hostDataViewer->removeNameFromFilter(previousItem);
    }
//    previousItem.clear();
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
