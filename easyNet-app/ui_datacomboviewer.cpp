#include "ui_datacomboviewer.h"
#include "objectcachefilter.h"
#include "objectupdater.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QScrollArea>

Ui_DataComboViewer::Ui_DataComboViewer(QWidget *parent)
    : Ui_DataViewer(parent)
{
}

Ui_DataComboViewer::~Ui_DataComboViewer()
{

}

QString Ui_DataComboViewer::currentItem()
{
    if (m_usePrettyNames)
        return comboBox->currentData(ObjectNameRole).toString();
    return comboBox->currentText();
}

void Ui_DataComboViewer::setCurrentItem(QString name)
{
    comboBox->setCurrentIndex(m_usePrettyNames ?
                                  comboBox->findData(name, ObjectNameRole) :
                                  comboBox->findText(name));
}

void Ui_DataComboViewer::addItem(QString name, QWidget *item)
{
    itemMap[name] = item;
    comboBox->addItem(name);
    scrollArea->setWidget(item);
    if (m_usePrettyNames)
    {
        comboBox->setItemData(comboBox->findText(name), name, ObjectNameRole); // pretty name will be substituted later
        itemDescriptionFilter->addName(name);
    }
}

void Ui_DataComboViewer::removeItem(QString name)
{
    comboBox->removeItem(m_usePrettyNames ?
                             comboBox->findData(name, ObjectNameRole) :
                             comboBox->findText(name));
    itemMap.remove(name);
    if (m_usePrettyNames)
        itemDescriptionFilter->removeName(name);
}

void Ui_DataComboViewer::createViewer()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    comboBox = new QComboBox(this);
    comboBox->setEditable(false);
    if (!m_usePrettyNames)
        comboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
    scrollArea = new QScrollArea(this);
    layout->addWidget(comboBox);
    layout->addWidget(scrollArea);
    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);
    setCentralWidget(widget);

    connect(comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [=](int index)
    {
        QString name = comboBox->itemData(index, ObjectNameRole).toString();
        scrollArea->setWidget(itemMap.value(name));
        emit currentItemChanged(name);
    });


}

void Ui_DataComboViewer::displayPrettyName(QString name)
{
    comboBox->setItemText(comboBox->findData(name, ObjectNameRole), prettyName.value(name));
    comboBox->model()->sort(0);
}

