#include "ui_datatabsviewer.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "enumclasses.h"
#include "sessionmanager.h"

#include <QVBoxLayout>

Ui_DataTabsViewer::Ui_DataTabsViewer()
    : Ui_DataViewer(), quiet_tab_change(false)
{
}

Ui_DataTabsViewer::~Ui_DataTabsViewer()
{
}

QString Ui_DataTabsViewer::currentItemName()
{
    return viewMap.key(tabWidget->currentWidget());
}

void Ui_DataTabsViewer::setCurrentItem(QString name)
{
    tabWidget->setCurrentWidget(viewMap.value(name));
}

QWidget *Ui_DataTabsViewer::currentView()
{
    return tabWidget->currentWidget();
}

void Ui_DataTabsViewer::addView(QString name, QWidget *view)
{
    viewMap[name] = view;
    if (m_usePrettyNames && SessionManager::instance()->exists(name))
    {
        itemDescriptionFilter->addName(name);
        tabWidget->insertTab(0, view, ""); // the (pretty) name on the tab will be set later
        itemDescriptionUpdater->requestObject(name); // could be a request from history widget
    }
    else
        tabWidget->insertTab(0, view, name);
}

QWidget *Ui_DataTabsViewer::takeView(QString name)
{
    QWidget *view = viewMap.value(name, nullptr);
    tabWidget->removeTab(tabWidget->indexOf(view));
    viewMap.remove(name);
    if (m_usePrettyNames)
        itemDescriptionFilter->removeName(name);
    return view;
}

//void Ui_DataTabsViewer::replaceItem(QString name, QWidget *item)
//{
//    eNerror << name;
//    QString current = currentItemName();
//    quiet_tab_change = true;
//    int index = tabWidget->indexOf(viewMap.value(name));
//    QString label = tabWidget->tabText(index);
//    tabWidget->insertTab(index, item, label);
//    tabWidget->removeTab(index + 1);
//    delete viewMap.value(name, nullptr);
//    viewMap[name] = item;
//    setCurrentItem(current);
//    quiet_tab_change = false;
//}

void Ui_DataTabsViewer::createViewer()
{
    tabWidget = new QTabWidget;
    tabWidget->setTabsClosable(true);
    setCentralWidget(tabWidget);
    connect(tabWidget, &QTabWidget::tabCloseRequested, [=](int index)
    {
        QString name = viewMap.key(tabWidget->widget(index));
        emit deleteItemRequested(name);
    });
    connect(tabWidget, &QTabWidget::currentChanged, [=](int index)
    {
        if (!quiet_tab_change)
            emit currentItemChanged(viewMap.key(tabWidget->widget(index)));
    });

}

void Ui_DataTabsViewer::displayPrettyName(QString name)
{
    tabWidget->setTabText(tabWidget->indexOf(viewMap.value(name)), prettyName.value(name));
}

