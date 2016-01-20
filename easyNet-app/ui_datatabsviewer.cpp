#include "ui_datatabsviewer.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "enumclasses.h"

#include <QVBoxLayout>

Ui_DataTabsViewer::Ui_DataTabsViewer()
    : Ui_DataViewer(), quiet_tab_change(false)
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
    if (itemMap.value(name, nullptr))
        replaceItem(name, item);
    else
    {
        // truly add
        itemMap[name] = item;
        if (m_usePrettyNames)
        {
            itemDescriptionFilter->addName(name);
            tabWidget->insertTab(0, item, ""); // the (pretty) name on the tab will be set later
        }
        else
            tabWidget->insertTab(0, item, name);
    }
}

void Ui_DataTabsViewer::removeItem(QString name)
{
    // does not delete the item
    tabWidget->removeTab(tabWidget->indexOf(itemMap.value(name)));
    itemMap.remove(name);
    if (m_usePrettyNames)
        itemDescriptionFilter->removeName(name);
}

void Ui_DataTabsViewer::replaceItem(QString name, QWidget *item)
{
    QString current = currentItem();
    quiet_tab_change = true;
    int index = tabWidget->indexOf(itemMap.value(name));
    QString label = tabWidget->tabText(index);
    tabWidget->insertTab(index, item, label);
    tabWidget->removeTab(index + 1);
    delete itemMap.value(name, nullptr);
    itemMap[name] = item;
    setCurrentItem(current);
    quiet_tab_change = false;
}

void Ui_DataTabsViewer::createViewer()
{
    tabWidget = new QTabWidget;
    tabWidget->setTabsClosable(true);
    setCentralWidget(tabWidget);
    connect(tabWidget, &QTabWidget::tabCloseRequested, [=](int index)
    {
        QString name = itemMap.key(tabWidget->widget(index));
        emit deleteItemRequested(name);
    });
    connect(tabWidget, &QTabWidget::currentChanged, [=](int index)
    {
        if (!quiet_tab_change)
            emit currentItemChanged(itemMap.key(tabWidget->widget(index)));
    });

}

void Ui_DataTabsViewer::displayPrettyName(QString name)
{
    tabWidget->setTabText(tabWidget->indexOf(itemMap.value(name)), prettyName.value(name));
}

