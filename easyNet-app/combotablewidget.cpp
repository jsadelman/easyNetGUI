#include "combotablewidget.h"
#include "dataframemodel.h"
#include "trialdataframemodel.h"
#include "objectcachefilter.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QComboBox>
#include <QDebug>


ComboTableWidget::ComboTableWidget(QWidget *parent)
    : TableWidgetInterface(parent)
{
     buildWidget();
     connect(comboBox, SIGNAL(activated(QString)), this, SLOT(setModel(QString)));
}

QString ComboTableWidget::currentTable()
{
    return comboBox->currentText();
}

void ComboTableWidget::setCurrentTable(QString name)
{
    comboBox->setCurrentText(name);
    emit currentTableChanged(name);
}

void ComboTableWidget::addTable_impl(QString name)
{
    dataframeFilter->setName(name);
}

void ComboTableWidget::updateTable_impl(QAbstractItemModel *model)
{
    DataFrameModel *dFmodel = getDataFrameModel(model);
    if (!dFmodel)
    {
        qDebug() << "ERROR: ComboTableWidget::updateTable_impl cannot extract DataFrameModel from argument";
        return;
    }

    QString name = dFmodel->name();
    if (name.isEmpty())
    {
        qDebug() << "ERROR: ComboTableWidget::updateTable_impl empty DataFrameModel name";
        return;
    }

    if (!modelMap.contains(name))
    {
        qDebug() << "ERROR: ComboTableWidget::updateTable_impl model name unknown" << name;
        return;
    }

    bool isNewModel = (modelMap.value(name) == nullptr);
    if (isNewModel)
        comboBox->addItem(name);
    else
    {
        DataFrameModel *oldDFmodel = modelMap.value(name);
        if (!oldDFmodel)
            return;

        TrialDataFrameModel *oldTDFmodel = qobject_cast<TrialDataFrameModel *>(modelMap.value(name));
        if (oldTDFmodel)
            delete oldTDFmodel;

        delete oldDFmodel;
    }
    modelMap[name] = dFmodel;

    if (name == currentTable() || isNewModel)
    {
        setModel(name);
        comboBox->setCurrentText(name);
    }
}

void ComboTableWidget::deleteTable_impl(QString name)
{
    if (!modelMap.contains(name))
        return;

    DataFrameModel *dFmodel = getDataFrameModel(modelMap.value(name));
    if (!dFmodel)
        return;

    TrialDataFrameModel *tdFmodel = qobject_cast<TrialDataFrameModel *>(modelMap.value(name));
    if (tdFmodel)
        delete tdFmodel;

    delete dFmodel;
    modelMap.remove(name);
    comboBox->removeItem(comboBox->findText(name));
    emit tableDeleted(name);
}

void ComboTableWidget::buildWidget()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    comboBox = new QComboBox(this);
    comboBox->setEditable(false);
    view = new QTableView(this);
    layout->addWidget(comboBox);
    layout->addWidget(view);
    setLayout(layout);
}

void ComboTableWidget::setModel(QString name)
{
    if (!modelMap.value(name, nullptr))
        return;

    QItemSelectionModel *m = view->selectionModel();
    view->setModel(modelMap.value(name));
    delete m;
}
