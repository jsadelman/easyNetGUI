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
        return comboBox->currentData().toString();
    return comboBox->currentText();
}

void Ui_DataComboViewer::setCurrentItem(QString name)
{
    comboBox->setCurrentIndex(m_usePrettyNames ?
                                  comboBox->findData(name) :
                                  comboBox->findText(name));
}

void Ui_DataComboViewer::addItem(QString name, QWidget *item)
{
    itemMap[name] = item;
    if (m_usePrettyNames)
    {
        comboBox->addItem("", name);
        itemDescriptionFilter->addName(name);
    }
    else
        comboBox->addItem(name);
}

void Ui_DataComboViewer::removeItem(QString name)
{
    comboBox->removeItem(m_usePrettyNames ?
                             comboBox->findData(name) :
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
    scrollArea->setWidgetResizable(true);
    layout->addWidget(comboBox);
    layout->addWidget(scrollArea);
    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);
    setCentralWidget(widget);

    connect(comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [=](int index)
    {
        QString name = m_usePrettyNames ? comboBox->itemData(index).toString() : comboBox->itemText(index);
        // non-destructive item substitution
        scrollArea->takeWidget();
        scrollArea->setWidget(itemMap.value(name));
        emit currentItemChanged(name);
    });


}

void Ui_DataComboViewer::displayPrettyName(QString name)
{
    comboBox->setItemText(comboBox->findData(name), prettyName.value(name));
    comboBox->model()->sort(0);
}

