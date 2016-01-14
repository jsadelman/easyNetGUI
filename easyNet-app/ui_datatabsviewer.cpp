#include "ui_datatabsviewer.h"
#include "objectcachefilter.h"
#include "objectupdater.h"

#include <QVBoxLayout>

Ui_DataTabsViewer::Ui_DataTabsViewer(QWidget *parent)
    : Ui_DataViewer(parent)
{
}

Ui_DataTabsViewer::~Ui_DataTabsViewer()
{
}

QString Ui_DataTabsViewer::currentItem()
{
    return itemMap.key(tabWidget->currentWidget());
}

void Ui_DataTabsViewer::setCurrentItem(QString name)
{
    tabWidget->setCurrentWidget(itemMap.value(name));
}

void Ui_DataTabsViewer::addItem(QString name, QWidget *item)
{
    itemMap[name] = item;
    tabWidget->addTab(item, ""); // the (pretty) name on the tab will be set later
    itemDescriptionFilter->addName(name);
}

void Ui_DataTabsViewer::removeItem(QString name)
{
    tabWidget->removeTab(tabWidget->indexOf(itemMap.value(name)));
    itemMap.remove(name);
    if (m_usePrettyNames)
        itemDescriptionFilter->removeName(name);
}

void Ui_DataTabsViewer::createViewer()
{
    tabWidget = new QTabWidget;
    tabWidget->setTabsClosable(true);
    setCentralWidget(tabWidget);
    connect(tabWidget, &QTabWidget::tabCloseRequested, [=](int index)
    {
        emit deleteItemRequested(itemMap.key(tabWidget->widget(index)));
    });
    connect(tabWidget, &QTabWidget::currentChanged, [=](int index)
    {
        emit currentItemChanged(itemMap.key(tabWidget->widget(index)));
    });

}

void Ui_DataTabsViewer::displayPrettyName(QString name)
{
    tabWidget->setTabText(tabWidget->indexOf(itemMap.value(name)), prettyName.value(name));
}

