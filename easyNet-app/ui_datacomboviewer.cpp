#include "ui_datacomboviewer.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "enumclasses.h"
#include "sessionmanager.h"


#include <QVBoxLayout>
#include <QComboBox>
#include <QScrollArea>
#include <QStackedWidget>
#include <QLabel>

Ui_DataComboViewer::Ui_DataComboViewer()
    : Ui_DataViewer()
{
}

Ui_DataComboViewer::~Ui_DataComboViewer()
{

}

QString Ui_DataComboViewer::currentItemName()
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

QWidget *Ui_DataComboViewer::currentView()
{
    return stackedWidget->currentWidget();
}

void Ui_DataComboViewer::addView(QString name, QWidget *view)
{
    stackedWidget->addWidget(view);
    viewMap[name] = view;
    if (m_usePrettyNames)
    {
        if (SessionManager::instance()->exists(name))
        {
            comboBox->addItem("", name);
            itemDescriptionFilter->addName(name);
        }
        else
        {
            comboBox->addItem(name, name);
            comboBox->model()->sort(0);
        }
    }
    else
    {
        comboBox->addItem(name);
        comboBox->model()->sort(0);
    }
}

QWidget *Ui_DataComboViewer::takeView(QString name)
{
    comboBox->removeItem(m_usePrettyNames ?
                             comboBox->findData(name) :
                             comboBox->findText(name));
    if (m_usePrettyNames)
        itemDescriptionFilter->removeName(name);
    QWidget *view = viewMap.value(name, nullptr);
    viewMap.remove(name);
    return view;
}

void Ui_DataComboViewer::createViewer()
{
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    widget->setLayout(layout);
    comboBox = new QComboBox(this);
    comboBox->setEditable(false);
    if (!m_usePrettyNames)
        comboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
    stackedWidget = new QStackedWidget(this);
    layout->addWidget(comboBox);
    layout->addWidget(stackedWidget);
    setCentralWidget(widget);
    connect(comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [=](int index)
    {
        QString name = m_usePrettyNames ? comboBox->itemData(index).toString() : comboBox->itemText(index);
        stackedWidget->setCurrentWidget(viewMap.value(name));
        emit currentItemChanged(name);
    });
    addView("", new QWidget(this));
}

void Ui_DataComboViewer::displayPrettyName(QString name)
{
    comboBox->setItemText(comboBox->findData(name), prettyName.value(name));
    comboBox->model()->sort(0);
}

