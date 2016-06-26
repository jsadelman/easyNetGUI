#include "ui_datatabsviewer.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "enumclasses.h"
#include "sessionmanager.h"

#include <QVBoxLayout>
#include <QSvgWidget>
#include <QTabBar>

Ui_DataTabsViewer::Ui_DataTabsViewer()
    : Ui_DataViewer(), quiet_tab_change(false), tabsClosable(true)
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
    tabWidget->setCurrentWidget(viewMap.value(name, nullptr));
}

QWidget *Ui_DataTabsViewer::currentView()
{
    return tabWidget->currentWidget();
}


void Ui_DataTabsViewer::addView(QString name, QWidget *view)
{
    viewMap[name] = view;
    if (usePrettyNames() && SessionManager::instance()->exists(name))
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
    if (usePrettyNames())
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
    tabWidget->setTabsClosable(tabsClosable);
//    setCentralWidget(tabWidget);
    mainLayout->addWidget(tabWidget);
    connect(tabWidget, &QTabWidget::tabCloseRequested, [=](int index)
    {
        QString name = viewMap.key(tabWidget->widget(index));
        emit deleteItemRequested(name);
    });
    connect(tabWidget, &QTabWidget::currentChanged, [=](int index)
    {
        if (!quiet_tab_change)
        {
            emit currentItemChanged(viewMap.key(tabWidget->widget(index)));
        }
    });
}

void Ui_DataTabsViewer::displayPrettyName(QString name)
{
    tabWidget->setStyleSheet("QTabBar::tab { height: 30px; }");
    tabWidget->setIconSize(QSize(24,24));

    int index = tabWidget->indexOf(viewMap.value(name));
    tabWidget->setTabText(index, prettyName.value(name));
    qDebug() << name << " view is" << viewMap.value(name);
    QTabBar* bar=tabWidget->tabBar();
    if (qobject_cast<QSvgWidget*>(viewMap.value(name))!=NULL)
    {
        // if (live && dependent on current trial)
        if (name.contains("letters"))
        {
            tabWidget->setTabIcon(tabWidget->indexOf(viewMap.value(name)), QIcon(":/images/graph-yellow2.png"));
            bar->setTabTextColor(index, Qt::black);
        }
        //      if (live && NOT dependent on current trial)
        else if (name.contains("words"))
        {
            tabWidget->setTabIcon(tabWidget->indexOf(viewMap.value(name)), QIcon(":/images/graph-green2.png"));
            bar->setTabTextColor(index, Qt::black);
        }
        //      if (dead)
        else
        {
            tabWidget->setTabIcon(tabWidget->indexOf(viewMap.value(name)), QIcon(":/images/graph-grey2.png"));
            bar->setTabTextColor(index, Qt::darkGray);
        }
    }
    else
    {
        // table

        // Note re colours: rather than traffic lights, I suggest the scale is based on *luminance*,
        // i.e., the brightest colour (yellow) indicates the most salient objects
//      if (live && dependent on current trial)
        if (name.contains("brief"))
            tabWidget->setTabIcon(tabWidget->indexOf(viewMap.value(name)), QIcon(":/images/table-yellow2.png"));
//      if (live && NOT dependent on current trial)
        else if (name.contains("present"))
            tabWidget->setTabIcon(tabWidget->indexOf(viewMap.value(name)), QIcon(":/images/table-green2.png"));
//      if (dead)
        else
            tabWidget->setTabIcon(tabWidget->indexOf(viewMap.value(name)), QIcon(":/images/table-grey2.png"));
    }
}

