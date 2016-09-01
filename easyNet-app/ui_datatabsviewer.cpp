#include "ui_datatabsviewer.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "enumclasses.h"
#include "sessionmanager.h"
#include "dataviewer.h"

#include <QVBoxLayout>
#include <QSvgWidget>
#include <QTabBar>
#include <QMenu>

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

QSize Ui_DataTabsViewer::frame()
{

    if (!tabWidget)
        return QSize();
    QWidget *dummy = Q_NULLPTR;
    if (tabWidget->count() == 0)
    {
        dummy = new QWidget();
        tabWidget->addTab(dummy, "");
        tabWidget->setCurrentWidget(dummy);
    }
    QSize s = tabWidget->size() - QSize(0,tabWidget->tabBar()->sizeHint().height());
    tabWidget->removeTab(tabWidget->indexOf(dummy));
    return s;

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

    tabWidget->setTabToolTip(0, name);
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
    tabWidget->setStyleSheet("QTabBar::tab { height: 30px; }");
    tabWidget->setIconSize(QSize(24,24));
    tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabWidget->tabBar(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    closeAllButThisTabAct = new QAction("close all but this tab", this);

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
    tabWidget->setTabText(tabWidget->indexOf(viewMap.value(name)), prettyName.value(name));
}

void Ui_DataTabsViewer::setStateIcon(QString name, int state)
{
    int index = tabWidget->indexOf(viewMap.value(name));
    if (index < 0)
        return;
    if (state == -1)
    {
        foreach (DataViewer * viewer, dataViewers)
            state = qMax(state, viewer->viewState(name));
    }
    int viewType = (SessionManager::instance()->descriptionCache->type(name) == "dataframe") ?
                ViewType_Table : ViewType_Plot;
    tabWidget->setTabIcon(index, SessionManager::instance()->viewIcon(viewType, state));
    switch (state)
    {
    case ViewState_Static:
        tabWidget->tabBar()->setTabTextColor(index, Qt::darkGray);
        break;
    case ViewState_Stale:
    case ViewState_Fresh:
        tabWidget->tabBar()->setTabTextColor(index, Qt::black);
        break;
    default:
        break;
    }
}

void Ui_DataTabsViewer::showContextMenu(const QPoint &point)
{
    if (point.isNull())
        return;
    int index = tabWidget->tabBar()->tabAt(point);
    if (index < 0 || index >= tabWidget->count())
        return;
    QMenu menu(this);
    if (tabWidget->count() > 1)
        menu.addAction(closeAllButThisTabAct);

    QAction *action = menu.exec(tabWidget->tabBar()->mapToGlobal(point));
    if (action == closeAllButThisTabAct)
        closeAllButThisTab(index);

}

void Ui_DataTabsViewer::closeAllButThisTab(int index)
{
    QStringList names;
    for (int i = 0; i < tabWidget->count(); ++i)
    {
        if (i != index)
            names.append(viewMap.key(tabWidget->widget(i)));
    }
    foreach (QString name, names)
    {
        emit deleteItemRequested(name);
    }
}
