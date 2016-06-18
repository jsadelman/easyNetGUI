#include "ui_dataviewer.h"
#include "enumclasses.h"
#include "sessionmanager.h"
#include "xmlaccessor.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "dataviewer.h"

#include <QAction>
#include <QSignalMapper>
#include <QShortcut>
#include <QDebug>
#include <QToolBar>
#include <QVBoxLayout>
#include <QDockWidget>

Ui_DataViewer::Ui_DataViewer()
    : QMainWindow(), m_usePrettyNames(false), setup(false), currentFileToolBar(0),
  currentEditToolBar(0),
  currentDispatchToolBar(0)
{
    mainWidget = new WidgetFwdResizeEvent;
    mainLayout = new QVBoxLayout;
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
}

Ui_DataViewer::~Ui_DataViewer()
{
}

void Ui_DataViewer::setupUi(DataViewer *dataViewer)
{
    if (!dataViewer)
    {
        eNerror << "invalid DataViewer" << dataViewer;
        return;
    }
    setParent(dataViewer);
    if (usePrettyNames())
    {
        itemDescriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
        itemDescriptionUpdater = new ObjectUpdater(this);
        itemDescriptionUpdater->setProxyModel(itemDescriptionFilter);
        connect(itemDescriptionUpdater, &ObjectUpdater::objectUpdated, [=](QDomDocument* description, QString name)
        {
            QDomElement rootElement = description->documentElement();
            QDomElement prettyNameElement = XMLAccessor::childElement(rootElement, "pretty name");
            QString pretty = XMLAccessor::value(prettyNameElement);
            prettyName.insert(name, pretty);
            displayPrettyName(name);
        });
    }
    if(!setup)     createViewer();
    createActions();
    createToolBars(dataViewer);

//    connect(setDispatchModeOverrideMapper, SIGNAL(mapped(int)),
//            dataViewer, SLOT(setDispatchModeOverride(int)));
//    connect(setDispatchModeAutoAct, SIGNAL(triggered(bool)),
//            dataViewer, SLOT(setDispatchModeAuto(bool)));

    connect(openAct, SIGNAL(triggered()), dataViewer, SLOT(open()));
    connect(saveAct, SIGNAL(triggered()), dataViewer, SLOT(save()));
    connect(copyAct, SIGNAL(triggered()), dataViewer, SLOT(copy()));
    connect(destroyAct, SIGNAL(triggered()), dataViewer, SLOT(destroySelectedItems()));
    connect(settingsAct, SIGNAL(triggered()), dataViewer, SIGNAL(showSettingsRequested()));
//    connect(findAct, SIGNAL(triggered()), dataViewer, SLOT(showFindDialog()));
//    connect(setDispatchModeAutoAct, SIGNAL(triggered(bool)),
//            dataViewer, SLOT(setDispatchModeAuto(bool)));
    connect(this, SIGNAL(deleteItemRequested(QString)), dataViewer, SLOT(initiateDestroyItem(QString)));
    connect(this, SIGNAL(currentItemChanged(QString)), dataViewer, SLOT(setCurrentItem(QString)));

//    dataViewer->setDispatchModeAuto(true);
//    setDispatchModeAutoAct->setChecked(true);
//    setDispatchModeAutoAct->setVisible(false); // will be set visible if the host viewer has a dispatcher
//    setDispatchModeOverrideActGroup->setVisible(false);

    setup=true;
}

void Ui_DataViewer::addHistoryWidget(QDockWidget *historyWidget)
{
    if (!historyAct)
    {
        eNerror << "cannot add a history widget before historyAct is created";
        return;
    }
    if (!historyWidget)
    {
        eNerror << "not a valid history widget";
        return;
    }
    connect(historyAct, SIGNAL(triggered(bool)), historyWidget, SLOT(setVisible(bool)));
    connect(historyAct, SIGNAL(triggered(bool)), this, SLOT(update()));
    historyWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    historyToolBar->show();
    historyWidget->setVisible(false);
    historyWidgets.append(historyWidget);
    if (historyWidget == historyWidgets.first())
    {
        addDockWidget(Qt::LeftDockWidgetArea, historyWidget);
    }
    else
    {
        splitDockWidget(historyWidgets.at(historyWidgets.indexOf(historyWidget) -1), historyWidget, Qt::Vertical);
    }
}

void Ui_DataViewer::createActions()
{
    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    copyAct = new QAction(QIcon(":/images/clipboard.png"), tr("&Copy to clipboard"), this);
    destroyAct = new QAction(QIcon(":/images/icon_trash.png"), "delete", this);
    destroyAct->setToolTip("delete current item");
    settingsAct = new QAction(QIcon(":/images/plot_settings.png"), tr("&Settings"), this);
    settingsAct->setToolTip(tr("Show settings form"));
    if(!setup)
    {
        historyAct = new QAction(QIcon(":/images/History.png"), tr("View list"), this);
        historyAct->setToolTip("show/hide view list");
        historyAct->setCheckable(true);
    }
}

void Ui_DataViewer::createToolBars(DataViewer*dv)
{
    if(!setup)
    {
        historyToolBar = addToolBar(tr("History"));
        historyToolBar->addAction(historyAct);
        historyToolBar->hide();
    }
    actionSet[dv]={saveAct,copyAct,destroyAct,settingsAct};
    fileToolBar[dv] = addToolBar(tr("File"));
    fileToolBar[dv]->addAction(openAct);
    fileToolBar[dv]->addAction(saveAct);
    fileToolBar[dv]->hide();

    editToolBar[dv] = addToolBar(tr("Edit"));
    editToolBar[dv]->addAction(copyAct);
    editToolBar[dv]->addAction(destroyAct);
    editToolBar[dv]->addAction(settingsAct);
    editToolBar[dv]->hide();

    dispatchToolBar[dv] = addToolBar(tr("Dispatch Mode"));
    dispatchToolBar[dv]->hide();
    setToolBars(dv);
}

void Ui_DataViewer::setToolBars(DataViewer*dv)
{
    if(currentDispatchToolBar) currentDispatchToolBar->hide();
    if(currentEditToolBar) currentEditToolBar->hide();
    if(currentFileToolBar) currentFileToolBar->hide();
    currentDispatchToolBar=dispatchToolBar[dv];
    currentEditToolBar=editToolBar[dv];
    currentFileToolBar=fileToolBar[dv];
    currentDispatchToolBar->show();
    currentEditToolBar->show();
    currentFileToolBar->show();
}
