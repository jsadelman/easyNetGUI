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
#include <QModelIndex>

DataViewerDispatcher::DataViewerDispatcher(DataViewer *host)
    : QObject(host), hostDataViewer(host), dispatchModeOverride(-1),
      dispatchModeAuto(true), previousDispatchMode(-1), previousItem(""),
      update_view_disabled(false), trialRunMode(TrialRunMode_Single),
      previousDispatchOverrideMode(-1)
{
    if (!host)
    {
        eNerror << "invalid host DataViewer, host is" << host;
    }
    host->setDispatcher(this);


    dispatchDefaultMode.insert(trialRunModeName.value(TrialRunMode_Single), -1);
    dispatchDefaultMode.insert(trialRunModeName.value(TrialRunMode_List), -1);

    dispatchModeFST.insert(qMakePair(Dispatch_New,Dispatch_New), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_New,Dispatch_Overwrite), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_New,Dispatch_Append), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_Overwrite,Dispatch_New), Dispatch_Overwrite);
    dispatchModeFST.insert(qMakePair(Dispatch_Overwrite,Dispatch_Overwrite), Dispatch_Overwrite);
    dispatchModeFST.insert(qMakePair(Dispatch_Overwrite,Dispatch_Append), Dispatch_Overwrite);
    dispatchModeFST.insert(qMakePair(Dispatch_Append,Dispatch_New), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_Append,Dispatch_Overwrite), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_Append,Dispatch_Append), Dispatch_Append);

    createHistory();
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

void DataViewerDispatcher::removeFromHistory(QString name)
{
    historyModel->removeView(name, trial(name));
    trialRunInfoMap.remove(name);
}


bool DataViewerDispatcher::inHistory(QString name)
{
    return historyModel->containsView(name, trial(name));
}

void DataViewerDispatcher::setInView(QString name, bool inView)
{
    historyModel->setInView(name, trial(name), inView);
}

void DataViewerDispatcher::setTrialRunMode(int mode)
{
    trialRunMode = mode;
    if (!dispatchModeAuto)
        restoreOverrideDefaultValue();
}

void DataViewerDispatcher::restoreOverrideDefaultValue()
{
    int mode = dispatchDefaultMode.value(trialRunModeName.value(trialRunMode));
    hostDataViewer->ui->setDispatchModeOverrideActs.at(mode)->setChecked(true);
    dispatchModeOverride = mode;
}

void DataViewerDispatcher::createHistory()
{
    historyModel = new HistoryTreeModel(this);
    historyWidget = new HistoryWidget;
    historyWidget->setModel(historyModel);
    historyWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    hostDataViewer->ui->addDockWidget(Qt::LeftDockWidgetArea, historyWidget);
    connect(historyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(updateView(QModelIndex,QModelIndex,QVector<int>)));
    connect(historyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            historyWidget->view, SLOT(expand(QModelIndex)));
    connect(historyWidget, SIGNAL(clicked(QString)), hostDataViewer->ui, SLOT(setCurrentItem(QString)));
    connect(hostDataViewer->ui, &Ui_DataViewer::currentItemChanged, [=](QString name)
    {
        historyWidget->view->setCurrentIndex(historyModel->viewIndex(name));
    });

    historyAct = historyWidget->toggleViewAction();
    historyAct->setIcon(QIcon(":/images/History.png"));
    historyAct->setText("History");
    historyAct->setToolTip("show/hide history");
    connect(historyAct, SIGNAL(triggered(bool)), historyWidget, SLOT(setVisible(bool)));
    historyWidget->setVisible(false);
    hostDataViewer->ui->dispatchToolBar->addAction(historyAct);
}

void DataViewerDispatcher::destroySelectedItems()
{
    foreach(QModelIndex index, historyWidget->view->selectionModel()->selectedIndexes())
    {
        if (index.parent().isValid())
        {
            QString name = historyModel->data(index, Qt::DisplayRole).toString();
            hostDataViewer->initiateDestroyItem(name);
        }
    }
}

void DataViewerDispatcher::updateView(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles)
{
//    if (update_view_disabled)
//        return;
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
//    update_view_disabled = true;
    disconnect(historyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(updateView(QModelIndex,QModelIndex,QVector<int>)));
    historyModel->removeView(item, trial(item));
    historyModel->appendView(item, TrialRunInfo(info).trial, inView);
    connect(historyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(updateView(QModelIndex,QModelIndex,QVector<int>)));
//    update_view_disabled = false;
}

//void DataViewerDispatcher::removeViews(QModelIndex index, int first, int last)
//{
//    if (!index.isValid() || index.parent().isValid())
//        return;

//    for (int row = first; row <= last; ++row)
//    {
//        QString name = historyModel->data(historyModel->index(row, 0, index), Qt::DisplayRole).toString();
//        hostDataViewer->initiateRemoveItem(name);
//    }
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
