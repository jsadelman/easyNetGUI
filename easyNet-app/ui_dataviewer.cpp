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

Ui_DataViewer::Ui_DataViewer()
    : QMainWindow(), m_usePrettyNames(false), setup(false)
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
    if (m_usePrettyNames)
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
    createToolBars();

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


void Ui_DataViewer::createActions()
{
//    dispatchModeName.insert(Dispatch_New, "New Page");
//    dispatchModeName.insert(Dispatch_Overwrite, "Overwrite");
//    dispatchModeName.insert(Dispatch_Append, "Append");

//    dispatchModeIconName.insert(Dispatch_New, ":/images/tab_new.png");
//    dispatchModeIconName.insert(Dispatch_Overwrite, ":/images/overwrite.png");
//    dispatchModeIconName.insert(Dispatch_Append, ":/images/append.png");

//    setDispatchModeOverrideMapper = new QSignalMapper(this);
//    setDispatchModeOverrideActGroup = new QActionGroup(this);
//    for (int mode = 0; mode < MAX_DISPATCH_MODE; ++mode)
//    {
//        setDispatchModeOverrideActs.insert(mode, new QAction(
//                                               QIcon(dispatchModeIconName.value(mode)),
//                                               dispatchModeName.value(mode),
//                                               this));
//        setDispatchModeOverrideActs.at(mode)->setCheckable(true);
//        setDispatchModeOverrideMapper->setMapping(setDispatchModeOverrideActs.at(mode), mode);
//        connect(setDispatchModeOverrideActs.at(mode), SIGNAL(triggered()),
//                setDispatchModeOverrideMapper, SLOT(map()));
//        setDispatchModeOverrideActGroup->addAction(setDispatchModeOverrideActs.at(mode));
//    }

//    setDispatchModeAutoAct = new QAction(QIcon(":/images/auto_icon.png"), "Auto", this);
//    setDispatchModeAutoAct->setToolTip("Override default page behaviour");
//    setDispatchModeAutoAct->setCheckable(true);

    qDebug() << "Defining openAct" << this;
    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
//    openAct->setShortcuts(QKeySequence::Open);
    // shortcuts don't work  here due to ambiguity across the different ui_dataviewer instances

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
//    saveAct->setShortcuts(QKeySequence::Save);

    copyAct = new QAction(QIcon(":/images/clipboard.png"), tr("&Copy to clipboard"), this);
//    copyAct->setShortcuts(QKeySequence::Copy);

    destroyAct = new QAction(QIcon(":/images/icon_trash.png"), "delete", this);
    destroyAct->setToolTip("delete current item");

    settingsAct = new QAction(QIcon(":/images/plot_settings.png"), tr("&Settings"), this);
    settingsAct->setToolTip(tr("Show settings form"));

}

void Ui_DataViewer::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(copyAct);
    editToolBar->addAction(destroyAct);
    editToolBar->addAction(settingsAct);

    dispatchToolBar = addToolBar(tr("Dispatch Mode"));
//    dispatchToolBar->addActions(setDispatchModeOverrideActs);
//    dispatchToolBar->addAction(setDispatchModeAutoAct);

}

