#include "tabstablewidget.h"
#include "dataframemodel.h"
#include "trialdataframemodel.h"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QTableView>

TabsTableWidget::TabsTableWidget(QWidget *parent)
    : TableWidgetInterface(parent)
{
    buildWidget();
}

QString TabsTableWidget::currentTable()
{
    QTableView *view = qobject_cast<QTableView*>(tabWidget->currentWidget());
    if (!view)
        return QString();

    DataFrameModel *dFmodel = getDataFrameModel(view->model());
    if (!dFmodel)
        return QString();

    return dFmodel->name();
}

void TabsTableWidget::setCurrentTable(QString name)
{
    if (!modelMap.contains(name))
        return;

    DataFrameModel *dFmodel = getDataFrameModel(modelMap.value(name));
    if (!dFmodel)
        return;

    tabWidget->setCurrentWidget(dFmodel->view());
}


void TabsTableWidget::updateTable_impl(QAbstractItemModel *model)
{
    DataFrameModel *dFmodel = getDataFrameModel(model);
    if (!dFmodel)
        return;

    QString name = dFmodel->name();
    if (name.isEmpty())
        return;

    bool isNewModel = !modelMap.contains(name);
    if (isNewModel)
    {
        dFmodel->setView(new QTableView(this));
        dFmodel->view()->setModel(model);
        tabWidget->addTab(dFmodel->view(), name);
    }
    else
    {
        DataFrameModel *oldDFmodel = getDataFrameModel(modelMap.value(name));
        TrialDataFrameModel *oldTDFmodel = qobject_cast<TrialDataFrameModel *>(modelMap.value(name));
        if (oldTDFmodel)
            delete oldTDFmodel;

        QTableView *view = oldDFmodel->view();
        dFmodel->setView(view);

        QItemSelectionModel *m = view->selectionModel();
        delete oldDFmodel;
        delete m;

        view->setModel(model);
    }
    modelMap[name] = model;
    if (isNewModel)
        setCurrentTable(name);
}

void TabsTableWidget::deleteTable_impl(QString name)
{
    if (!modelMap.contains(name))
        return;

    DataFrameModel *dFmodel = getDataFrameModel(modelMap.value(name));
    if (!dFmodel)
        return;

    TrialDataFrameModel *tdFmodel = qobject_cast<TrialDataFrameModel *>(modelMap.value(name));

    QTableView *view = dFmodel->view();
    int index = tabWidget->indexOf(view);
    tabWidget->removeTab(index);
    delete view;
    if (tdFmodel)
        delete tdFmodel;

    delete dFmodel;
    modelMap.remove(name);
}

void TabsTableWidget::buildWidget()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    tabWidget = new QTabWidget;
    layout->addWidget(tabWidget);
    setLayout(layout);
}

