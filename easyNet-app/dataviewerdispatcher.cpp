#include "dataviewerdispatcher.h"
#include "ui_dataviewer.h"
#include "enumclasses.h"
#include "xmlaccessor.h"
#include "dataviewer.h"
#include "historytreemodel.h"
#include "historywidget.h"
#include "sessionmanager.h"
#include "xmlform.h"
#include "xmlmodel.h"
#include "lazynutobjectmodel.h"
#include "settingsform.h"
#include "settingsformdialog.h"

#include <QAction>
#include <QToolBar>
#include <QTreeView>
#include <QModelIndex>
#include <QScrollArea>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QDialog>


Q_DECLARE_METATYPE(QSharedPointer<QDomDocument> )


DataViewerDispatcher::DataViewerDispatcher(DataViewer *host)
    : QObject(host),
      hostDataViewer(host),
      previousDispatchMode(-1),
      currentDispatchAction(-1),
      previousItem(""),
      infoIsVisible(false),
      trialRunMode(TrialRunMode_Single),
      m_snapshotActive(true),
      m_copyDfActive(false)
{
    if (!host)
    {
        eNerror << "invalid host DataViewer, host is" << host;
    }
    host->setDispatcher(this);

    dispatchDefaultMode.insert(trialRunModeName.value(TrialRunMode_Single), -1);
    dispatchDefaultMode.insert(trialRunModeName.value(TrialRunMode_List), -1);
    dispatchModeText.insert(Dispatch_New, "new");
    dispatchModeText.insert(Dispatch_Append, "append");
    dispatchModeText.insert(Dispatch_Overwrite, "overwrite");

    createHistoryWidget();
//    createInfoWidget();
}

DataViewerDispatcher::~DataViewerDispatcher()
{
}

QString DataViewerDispatcher::trial(QString name)
{
    QList<QSharedPointer<QDomDocument> > info = SessionManager::instance()->trialRunInfo(name);
    return info.isEmpty() ? no_trial : TrialRunInfo(info.last()).trial  ;
}

QString DataViewerDispatcher::runMode(QString name)
{
    QList<QSharedPointer<QDomDocument> > info = SessionManager::instance()->trialRunInfo(name);
    return !info.isEmpty() && info.last() ? TrialRunInfo(info.last()).runMode : QString();
}

QString DataViewerDispatcher::results(QString name)
{
    QList<QSharedPointer<QDomDocument> > info = SessionManager::instance()->trialRunInfo(name);
    return !info.isEmpty() && info.last() ? TrialRunInfo(info.last()).results : QString();
}

void DataViewerDispatcher::addToHistory(QString name, bool inView)
{
    if (inHistory(name))
    {
        eNwarning << QString("attempt to add item %1 to History, the item is in History already").arg(name);
        return;
    }
    historyModel->appendView(name, trial(name), inView);
}

void DataViewerDispatcher::removeFromHistory(QString name)
{
    historyModel->removeView(name);
    SessionManager::instance()->removeTrialRunInfo(name);
}


bool DataViewerDispatcher::inHistory(QString name)
{
    return historyModel->containsView(name);
}

bool DataViewerDispatcher::isInView(QString name)
{
    return historyModel->isInView(name);
}

void DataViewerDispatcher::setInView(QString name, bool inView)
{
    historyModel->setInView(name, trial(name), inView);
//    SessionManager::instance()->setShowHint(name, inView ? "1" : "0");
}

void DataViewerDispatcher::setTrialRunMode(int mode)
{
    trialRunMode = mode;
//    if (!dispatchModeAuto)
//        restoreOverrideDefaultValue();
}

void DataViewerDispatcher::createHistoryWidget()
{
    historyModel = new HistoryTreeModel(this);
    historyWidget = new HistoryWidget(hostDataViewer->ui, hostDataViewer->name());
    historyWidget->setModel(historyModel);
//    historyWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
//    hostDataViewer->ui->addDockWidget(Qt::LeftDockWidgetArea, historyWidget);
    connect(historyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(updateView(QModelIndex,QModelIndex,QVector<int>)));
    connect(historyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            historyWidget->view, SLOT(expand(QModelIndex)));
    connect(historyWidget, SIGNAL(clicked(QString)), hostDataViewer->ui, SLOT(setCurrentItem(QString)));
    connect(hostDataViewer->ui, &Ui_DataViewer::currentItemChanged, [=](QString name)
    {
        historyWidget->view->setCurrentIndex(historyModel->viewIndex(name));
    });
//    connect(historyWidget, &HistoryWidget::clicked, [=](QString name)
//    {
//        showInfo(true, name);
//    });


//    connect(hostDataViewer->ui->historyAct, SIGNAL(triggered(bool)), historyWidget, SLOT(setVisible(bool)));
//    hostDataViewer->ui->historyToolBar->show();
//    historyWidget->setVisible(false);
    hostDataViewer->ui->addHistoryWidget(historyWidget);

}


void DataViewerDispatcher::createInfoWidget()
{
    infoDock = new QDockWidget(hostDataViewer->ui);
    infoDock->setWindowTitle("Trial run info");
    infoDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    infoScroll = new QScrollArea(hostDataViewer->ui);
    infoDock->setWidget(infoScroll);
//    infoScroll->setWidgetResizable(true);
    hostDataViewer->ui->addDockWidget(Qt::LeftDockWidgetArea, infoDock);
    connect(hostDataViewer->ui, &Ui_DataViewer::currentItemChanged, [=]()
    {
        if (infoAct->isChecked())
            showInfo(true);
    });
    infoAct = infoDock->toggleViewAction();
    infoAct->setIcon(QIcon(":/images/Information-icon.png"));
    infoAct->setText("Info");
    infoAct->setToolTip("show/hide trial run info");
    connect(infoAct, SIGNAL(triggered(bool)), this, SLOT(showInfo(bool)));
    infoDock->setVisible(false);
}

void DataViewerDispatcher::addExtraActions()
{
    preferencesAct = new QAction(QIcon(":/images/setting.png"), "Preferences", this);
    preferencesAct->setToolTip("Show preferences for this viewer");
    connect(preferencesAct, SIGNAL(triggered()), this, SLOT(showPreferences()));
    hostDataViewer->ui->dispatchToolBar[hostDataViewer]->addAction(preferencesAct);
}

void DataViewerDispatcher::destroySelectedItems()
{
    foreach(QModelIndex index, historyWidget->view->selectionModel()->selectedIndexes())
    {
        if (index.parent().isValid())
        {
            QString name = historyModel->data(index, NameRole).toString();
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
            name        = historyModel->data(historyModel->index(row, 0, topLeft.parent()), NameRole).toString();
            checked     = historyModel->data(historyModel->index(row, 0, topLeft.parent()), Qt::CheckStateRole).toInt();
//            qDebug() << Q_FUNC_INFO << name << checked;
            SessionManager::instance()->setShowHint(name, checked == Qt::Checked ? "1" : "0");
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

void DataViewerDispatcher::updateHistory(QString name)
{
    QList<QSharedPointer<QDomDocument> > info = SessionManager::instance()->trialRunInfo(name);
    QString newTrial = info.isEmpty() ? no_trial : TrialRunInfo(info.last()).trial;
    if (historyModel->trial(name) == newTrial) // old trial == new trial
        return;

    bool isInView = historyModel->isInView(name);
    disconnect(historyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(updateView(QModelIndex,QModelIndex,QVector<int>)));
    historyModel->removeView(name, historyModel->trial(name));
    historyModel->appendView(name, newTrial);
    historyModel->setInView(name, newTrial, isInView);
    connect(historyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(updateView(QModelIndex,QModelIndex,QVector<int>)));
}


void DataViewerDispatcher::showInfo(bool show, QString name)
{
//  deactivated
    return;
    QVBoxLayout *infoLayout;
    QWidget *infoWidget = infoScroll->takeWidget();
    if (infoWidget)
    {
        infoLayout = qobject_cast<QVBoxLayout *>(infoWidget->layout());
        QLayoutItem *item;
        while ((item = infoLayout->takeAt(0)) != 0)
        {
            QTreeView *infoView = qobject_cast<QTreeView *>(item->widget());
            if (infoView)
            {
                delete infoView->model();
                delete infoView;
            }
        }
    }
    delete infoWidget;
    if (show)
    {
        infoWidget = new QWidget;
        infoLayout = new QVBoxLayout;
        infoLayout->setSizeConstraint(QLayout::SetFixedSize);
        infoWidget->setLayout(infoLayout);

        foreach(QSharedPointer<QDomDocument> info, SessionManager::instance()->trialRunInfo(name.isEmpty() ? hostDataViewer->ui->currentItemName() : name))
            if (info)
            {
                XMLModel *infoModel = new XMLModel(info, this);
                QTreeView *infoView = new QTreeView();
                infoView->setModel(infoModel);
                infoView->header()->setStretchLastSection(true);
                infoView->setEditTriggers(QAbstractItemView::NoEditTriggers);
                infoView->setSelectionMode(QAbstractItemView::NoSelection);
                infoView->expandAll();
                infoView->resizeColumnToContents(0);
                infoView->resizeColumnToContents(1);
                infoView->setMinimumWidth(infoView->width());
                infoView->setHeaderHidden(true);
                infoView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                //            infoView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                //            infoView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                infoLayout->addWidget(infoView);
            }
        infoScroll->setWidget(infoWidget);
        infoScroll->adjustSize();
        infoDock->setVisible(true);
    }
    else
    {
        infoDock->setVisible(false);
    }
    infoIsVisible = show;
}

void DataViewerDispatcher::updateInfo(QString name)
{
    if (infoIsVisible && SessionManager::instance()->dependencies(name).contains(hostDataViewer->ui->currentItemName()))
        showInfo(true);
}

void DataViewerDispatcher::enableActions(bool enable)
{
    preferencesAct->setEnabled(enable);
}

void DataViewerDispatcher::showPreferences()
{
    QDomDocument *preferencesDomDoc = makePreferencesDomDoc();
    SettingsForm *form = new SettingsForm(preferencesDomDoc);
    SettingsFormDialog dialog(preferencesDomDoc, form, "Viewer Preferences");
    dialog.build();
    int result = dialog.exec();
    if (result == QDialog::Accepted)
    {
        QMapIterator<QString, QString> settings_it(dialog.settings);
        while (settings_it.hasNext())
        {
            settings_it.next();
            if (dispatchDefaultMode.contains(settings_it.key()))
                dispatchDefaultMode[settings_it.key()] = dispatchModeText.key(settings_it.value());
            else if (settings_it.key().contains("snapshot", Qt::CaseInsensitive))
                setSnapshotActive(settings_it.value() == "yes");
            else if (settings_it.key().contains("Copy source dataframes", Qt::CaseInsensitive))
                setCopyDfActive(settings_it.value() == "yes");
        }
    }
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
