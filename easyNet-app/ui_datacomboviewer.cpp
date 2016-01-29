#include "ui_datacomboviewer.h"
#include "objectcachefilter.h"
#include "objectupdater.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QScrollArea>
#include <QLabel>

Ui_DataComboViewer::Ui_DataComboViewer()
    : Ui_DataViewer()
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
    if (itemMap.value(name, nullptr))
        replaceItem(name, item);
    else
    {
        // truly add
        itemMap[name] = item;
        if (m_usePrettyNames)
        {
            comboBox->addItem("", name);
            itemDescriptionFilter->addName(name);
        }
        else
        {
            comboBox->addItem(name);
            comboBox->model()->sort(0);
        }
    }
}

void Ui_DataComboViewer::removeItem(QString name)
{
    if (!itemMap.contains(name))
        return;
    comboBox->removeItem(m_usePrettyNames ?
                             comboBox->findData(name) :
                             comboBox->findText(name));
    itemMap.remove(name);
    if (m_usePrettyNames)
        itemDescriptionFilter->removeName(name);
}

void Ui_DataComboViewer::replaceItem(QString name, QWidget *item)
{
    delete itemMap.value(name);
    itemMap[name] = item;
    if (currentItem() == name)
        scrollArea->setWidget(item);
}

void Ui_DataComboViewer::createViewer()
{
    QWidget *widget = new QWidget;

    QVBoxLayout *layout = new QVBoxLayout;
    widget->setLayout(layout);

    comboBox = new QComboBox(this);
    comboBox->setEditable(false);
    itemMap.insert("", new QWidget(this));


    if (!m_usePrettyNames)
        comboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(comboBox);
    layout->addWidget(scrollArea);

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

    comboBox->addItem("", "");
}

void Ui_DataComboViewer::displayPrettyName(QString name)
{
    comboBox->setItemText(comboBox->findData(name), prettyName.value(name));
    comboBox->model()->sort(0);
}

