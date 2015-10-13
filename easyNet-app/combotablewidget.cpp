#include "combotablewidget.h"
#include "dataframemodel.h"
#include "trialdataframemodel.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QComboBox>


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
}

void ComboTableWidget::updateTable_impl(QAbstractItemModel *model)
{
    DataFrameModel *dFmodel = getDataFrameModel(model);
    if (!dFmodel)
        return;

    QString name = dFmodel->name();
    if (name.isEmpty())
        return;

    bool isNewModel = !modelMap.contains(name);
    if (isNewModel)
        comboBox->addItem(name);
    else
    {
        DataFrameModel *oldDFmodel = getDataFrameModel(modelMap.value(name));
        if (!oldDFmodel)
            return;

        TrialDataFrameModel *oldTDFmodel = qobject_cast<TrialDataFrameModel *>(modelMap.value(name));
        if (oldTDFmodel)
            delete oldTDFmodel;

        delete oldDFmodel;
    }
    modelMap[name] = model;

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
