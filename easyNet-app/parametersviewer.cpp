#include "parametersviewer.h"
#include "parametersmodel.h"
#include "ui_dataviewer.h"
#include "xmlelement.h"
#include "sessionmanager.h"
#include "easyNetMainWindow.h"
#include "parametersproxymodel.h"
#include "lazynutjob.h"


#include <QToolButton>
#include <QTableView>


ParametersViewer::ParametersViewer(Ui_DataViewer *ui, QWidget *parent)
    : DataframeViewer(ui, parent)
{
    setShowInMainViewer(false);
    setDragDropColumns(false);

    ui->settingsAct->setVisible(false);
    plotButton->setVisible(false);
    dataframeViewButton->setVisible(false);

    restoreButton = new QToolButton(this);
    restoreButton->setIcon(QIcon(":/images/restore.png"));
    restoreButton->setVisible(true);
    restoreButton->setEnabled(false);
    restoreButton->setPopupMode(QToolButton::InstantPopup);
    restoreButton->setToolTip("Restore parameters");
    restoreMenu = new QMenu(restoreButton);
    restoreSelectionAct = new QAction("Restore selected parameters", this);
    restoreSelectionAct->setEnabled(false);
    connect(restoreSelectionAct, SIGNAL(triggered()), this, SLOT(restoreSelection()));
    restoreAllAct = new QAction("Restore all parameters", this);
    connect(restoreAllAct, SIGNAL(triggered()), this, SLOT(restoreAll()));
    restoreMenu->addActions(QList<QAction *>({restoreSelectionAct, restoreAllAct}));
    connect(restoreMenu, &QMenu::aboutToShow, [=]()
    {
        QAbstractItemView *currentView = qobject_cast<QAbstractItemView *>(ui->currentView());
        restoreSelectionAct->setEnabled(currentView && currentView->selectionModel()->hasSelection());
    });
    restoreButton->setMenu(restoreMenu);
    ui->editToolBar[this]->addWidget(restoreButton);
}

void ParametersViewer::destroyItem_impl(QString name)
{
    referenceProxyMap.remove(name);
    DataframeViewer::destroyItem_impl(name);
}

void ParametersViewer::enableActions(bool enable)
{
    DataframeViewer::enableActions(enable);
    restoreButton->setEnabled(enable);
}

void ParametersViewer::updateDataframe(QDomDocument *domDoc, QString name)
{
    if (!modelMap.contains(name))
    {
        eNerror << QString("attempt to update non-existing dataframe %1").arg(name);
        return;
    }
    ParametersModel *dfModel = new ParametersModel(domDoc, this);
    dfModel->setName(name);
    connect(dfModel, SIGNAL(newParamValueSig(QString,QString)),
            this, SLOT(setParameter(QString,QString)));
    if (referenceProxyMap.value(name, nullptr))
        referenceProxyMap.value(name)->setReferenceModel(dfModel);

    bool isNew = !modelMap.value(name, nullptr);
    QTableView *view = qobject_cast<QTableView*>(ui->view(name));
    if (isNew && view)
    {
        view->setEditTriggers(QAbstractItemView::AllEditTriggers);
    }
    else if (!isNew)
    {
        DataFrameModel *oldDFmodel = modelMap.value(name);
        QItemSelectionModel *m = view ? view->selectionModel() : nullptr;
        delete oldDFmodel;
        delete m;
    }
    modelMap[name] = dfModel;
    limitedGet(name, maxFirstDisplayCells);
    if (view)
    {
        QString ref = reference(name);
        if (!ref.isEmpty() && !contains(ref))
            addItem(ref, true);
        ParametersProxyModel *proxy = new ParametersProxyModel(modelMap[name]);
        proxy->setSourceModel(modelMap[name]);
        if (modelMap.value(ref, nullptr))
            proxy->setReferenceModel(modelMap.value(ref));
        if (!ref.isEmpty())
            referenceProxyMap[ref] = proxy;
        view->setModel(proxy);
        view->verticalHeader()->show();
        view->horizontalHeader()->show();
        view->resizeColumnsToContents();
    }
    enableActions(true);
}

void ParametersViewer::restoreSelection()
{
    QString paramSet = ui->currentItemName();
    ParametersModel *paramModel = qobject_cast<ParametersModel *>(modelMap.value(paramSet));
    ParametersModel *refModel = qobject_cast<ParametersModel *>(modelMap.value(reference(paramSet)));
    if (!(refModel && paramModel))
        return;
    QTableView *view = qobject_cast<QTableView*>(ui->view(paramSet));
    if (!view)
        return;
    QItemSelectionModel *selectionModel = view->selectionModel();
    if (!selectionModel || !selectionModel->hasSelection())
        return;
    LazyNutJob *job = new LazyNutJob;
    foreach(QModelIndex index, selectionModel->selectedIndexes())
    {
        QString param = paramModel->data(paramModel->index(index.row(),0)).toString();
        QString value = refModel->value(param);
        if (!value.isEmpty())
            job->cmdList << QString("%1 set %2 %3").arg(paramSet).arg(param).arg(value);
    }
    if (job->cmdList.isEmpty())
        delete job;
    else
    {
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job
                << SessionManager::instance()->recentlyModifiedJob();
        SessionManager::instance()->submitJobs(jobs);
    }
}

void ParametersViewer::restoreAll()
{
    QString paramSet = ui->currentItemName();
    ParametersModel *paramModel = qobject_cast<ParametersModel *>(modelMap.value(paramSet));
    ParametersModel *refModel = qobject_cast<ParametersModel *>(modelMap.value(reference(paramSet)));
    if (!(refModel && paramModel))
        return;

    LazyNutJob *job = new LazyNutJob;
    for (int row = 0; row < refModel->rowCount(); ++row)
    {
        QString param = refModel->data(refModel->index(row, 0)).toString();
        QString value = refModel->value(param);
        if (!value.isEmpty())
            job->cmdList << QString("%1 set %2 %3").arg(paramSet).arg(param).arg(value);
    }
    if (job->cmdList.isEmpty())
        delete job;
    else
    {
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job
                << SessionManager::instance()->recentlyModifiedJob();
        SessionManager::instance()->submitJobs(jobs);
    }
}


QString ParametersViewer::reference(QString name)
{
    QDomDocument *description = SessionManager::instance()->description(name);
    if (description && !XMLelement(*description)["history"].listValues().isEmpty())
        return XMLelement(*description)["history"].listValues().first();
    return QString();
}
