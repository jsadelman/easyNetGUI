#include "dataviewerdispatcher.h"
#include "ui_dataviewer.h"
#include "enumclasses.h"
#include "xmlaccessor.h"
#include "dataviewer.h"
#include "historytreemodel.h"
#include "historywidget.h"
#include "sessionmanager.h"

#include <QAction>
#include <QToolBar>
#include <QTreeView>

DataViewerDispatcher::DataViewerDispatcher(DataViewer *host)
    : QObject(host), hostDataViewer(host), dispatchModeOverride(-1),
      dispatchModeAuto(true), previousDispatchMode(-1), previousItem(""), no_update_view(false)
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
    historyModel = new HistoryTreeModel(this);
    historyWidget = new HistoryWidget;
    historyWidget->setModel(historyModel);
    historyWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    host->ui->addDockWidget(Qt::LeftDockWidgetArea, historyWidget);
    connect(historyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(updateView(QModelIndex,QModelIndex,QVector<int>)));
    connect(historyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            historyWidget->view, SLOT(expand(QModelIndex)));

//    connect(historyWidget->moveToViewerAct, SIGNAL(triggered()), this, SLOT(moveFromHistoryToViewer()));
    connect(historyWidget->destroyAct, SIGNAL(triggered()), this, SLOT(removeFromHistory()));
//    connect(historyWidget, SIGNAL(clicked(QString)), this, SLOT(displayItemFromHistory(QString)));
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


void DataViewerDispatcher::setTrialRunInfo(QString item, QSharedPointer<QDomDocument> info)
{
    trialRunInfoMap.insert(item, info);
}

void DataViewerDispatcher::copyTrialRunInfo(QString fromItem, QString toItem)
{
    if (trialRunInfoMap.contains(fromItem))
        trialRunInfoMap.insert(toItem, trialRunInfoMap.value(fromItem));
}

QString DataViewerDispatcher::trial(QString name)
{
    if (trialRunInfoMap.contains(name))
        return  trialRunInfoMap.value(name) ? TrialRunInfo(trialRunInfoMap.value(name)).trial : no_trial;
    return QString();
}

QString DataViewerDispatcher::runMode(QString name)
{
    if (trialRunInfoMap.contains(name))
        return TrialRunInfo(trialRunInfoMap.value(name)).runMode;
    return QString();
}

QString DataViewerDispatcher::results(QString name)
{
    if (trialRunInfoMap.contains(name))
        return TrialRunInfo(trialRunInfoMap.value(name)).results;
    return QString();
}

void DataViewerDispatcher::addToHistory(QString name, bool inView, QSharedPointer<QDomDocument> info)
{
    if (trialRunInfoMap.contains(name))
    {
        eNwarning << QString("attempt to add item %1 to History, the item is in History already").arg(name);
        return;
    }
    trialRunInfoMap.insert(name, info);
    historyModel->appendView(name, trial(name), inView);
}

//void DataViewerDispatcher::moveFromHistoryToViewer()
//{
//    foreach(QString name, historyModel->selectedItems())
//    {
//        historyModel->destroy(name);
//        if (!hostDataViewer->ui->contains(name))
//        {
//            hostDataViewer->ui->addView(name, hostDataViewer->makeView());
//            if (!hostDataViewer->isLazy())
//                hostDataViewer->addNameToFilter(name);
//        }
//        hostDataViewer->ui->setCurrentItem(name);
//    }
//}

//void DataViewerDispatcher::moveFromViewerToHistory(QString name)
//{
//    historyModel->create(name);
//    hostDataViewer->ui->takeView(name);
//    if (!hostDataViewer->isLazy())
//        hostDataViewer->removeNameFromFilter(name);
//}

bool DataViewerDispatcher::inHistory(QString name)
{
    return historyModel->containsView(name, trial(name));
}

void DataViewerDispatcher::setInView(QString name, bool inView)
{
    historyModel->setInView(name, trial(name), inView);
}

void DataViewerDispatcher::removeFromHistory()
{
//    foreach(QString item, historyModel->selectedItems())
//    {
//        SessionManager::instance()->destroyObject(item);
//        historyModel->destroy(item);
//    }
}

//void DataViewerDispatcher::displayItemFromHistory(QString name)
//{
//    if (name.isEmpty())
//        return;
//    if (hostDataViewer->ui->contains(name))
//    {
//        if (hostDataViewer->ui->currentItemName() != name)
//        {
//            hostDataViewer->ui->setCurrentItem(name);
//        }
//        return;
//    }
//    removePreviousItem();
//    hostDataViewer->ui->addView(name, hostDataViewer->ui->view(name));
//    if (!hostDataViewer->isLazy())
//        hostDataViewer->addNameToFilter(name);
//    hostDataViewer->ui->setCurrentItem(name);
//    previousItem = name;
//}

void DataViewerDispatcher::setHistoryVisible(bool visible)
{
    historyWidget->setVisible(visible);
//    if (!visible)
//        removePreviousItem();

}

void DataViewerDispatcher::updateView(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles)
{
    if (no_update_view)
        return;
    if (! (topLeft.isValid() && bottomRight.isValid() &&
           topLeft.parent().isValid() && bottomRight.parent().isValid() &&
           topLeft.column() == 0 && bottomRight.column() == 0 &&
           topLeft.parent() == bottomRight.parent() &&
           topLeft.row() <= bottomRight.row()) )
    {
//        eNwarning << "index pair from historyModel is not admissible";
        return;
    }
    if (roles.contains(Qt::CheckStateRole))
    {
        QString name;
        int checked;
        for (int row = topLeft.row(); row <= bottomRight.row(); ++row)
        {
            name        = historyModel->data(historyModel->index(row, 0, topLeft.parent()), Qt::DisplayRole).toString();
            checked     = historyModel->data(historyModel->index(row, 0, topLeft.parent()), Qt::CheckStateRole).toInt();
            if (checked == Qt::Checked)
            {
                hostDataViewer->addView(name);
                hostDataViewer->ui->setCurrentItem(name);
            }
            else
            {
                hostDataViewer->removeView(name);
            }
        }
    }
    return;
}

void DataViewerDispatcher::updateHistory(QString item, QSharedPointer<QDomDocument> info)
{
    if (trial(item) == TrialRunInfo(info).trial) // old trial == new trial
        return;

    bool inView = historyModel->isInView(item, trial(item));
    no_update_view = true;
    historyModel->removeView(item, trial(item));
    historyModel->appendView(item, TrialRunInfo(info).trial, inView);
    no_update_view = false;
}

//void DataViewerDispatcher::removePreviousItem()
//{
//    if (historyModel->contains(previousItem))
//    {
//        hostDataViewer->ui->takeView(previousItem);
//        if (!hostDataViewer->isLazy())
//            hostDataViewer->removeNameFromFilter(previousItem);
//    }
////    previousItem.clear();
//}


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
