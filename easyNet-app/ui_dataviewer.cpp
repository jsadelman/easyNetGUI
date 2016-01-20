#include "ui_dataviewer.h"
#include "enumclasses.h"
#include "sessionmanager.h"
#include "xmlaccessor.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "dataviewer.h"



#include <QAction>
#include <QSignalMapper>
#include <QDebug>
#include <QToolBar>

Ui_DataViewer::Ui_DataViewer()
    : QMainWindow(), m_usePrettyNames(false)
{
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
    createViewer();
    createActions();
    createToolBars();

    connect(setDispatchModeOverrideMapper, SIGNAL(mapped(int)),
            dataViewer, SLOT(setDispatchModeOverride(int)));
    connect(setDispatchModeAutoAct, SIGNAL(triggered(bool)),
            dataViewer, SLOT(setDispatchModeAuto(bool)));

    connect(openAct, SIGNAL(triggered()), dataViewer, SLOT(open()));
    connect(saveAct, SIGNAL(triggered()), dataViewer, SLOT(save()));
    connect(copyAct, SIGNAL(triggered()), dataViewer, SLOT(copy()));
//    connect(findAct, SIGNAL(triggered()), dataViewer, SLOT(showFindDialog()));
    connect(setDispatchModeAutoAct, SIGNAL(triggered(bool)),
            dataViewer, SLOT(setDispatchModeAuto(bool)));
//    dataViewer->setDispatchModeAuto(true);
    setDispatchModeAutoAct->setChecked(true);
    setDispatchModeAutoAct->setVisible(false); // will be set visible if the host viewer has a dispatcher
    setDispatchModeOverrideActGroup->setVisible(false);
    findAct->setVisible(false); // visible only for dataframe views
    copyDFAct->setVisible(false); // visible only for dataframe views
}

void Ui_DataViewer::createActions()
{
    dispatchModeName.insert(Dispatch_New, "New Page");
    dispatchModeName.insert(Dispatch_Overwrite, "Overwrite");
    dispatchModeName.insert(Dispatch_Append, "Append");

    dispatchModeIconName.insert(Dispatch_New, ":/images/tab_new.png");
    dispatchModeIconName.insert(Dispatch_Overwrite, ":/images/overwrite.png");
    dispatchModeIconName.insert(Dispatch_Append, ":/images/append.png");

    setDispatchModeOverrideMapper = new QSignalMapper(this);
    setDispatchModeOverrideActGroup = new QActionGroup(this);
    for (int mode = 0; mode < MAX_DISPATCH_MODE; ++mode)
    {
        setDispatchModeOverrideActs.insert(mode, new QAction(
                                               QIcon(dispatchModeIconName.value(mode)),
                                               dispatchModeName.value(mode),
                                               this));
        setDispatchModeOverrideActs.at(mode)->setCheckable(true);
        setDispatchModeOverrideMapper->setMapping(setDispatchModeOverrideActs.at(mode), mode);
        connect(setDispatchModeOverrideActs.at(mode), SIGNAL(triggered()),
                setDispatchModeOverrideMapper, SLOT(map()));
        setDispatchModeOverrideActGroup->addAction(setDispatchModeOverrideActs.at(mode));
    }

    setDispatchModeAutoAct = new QAction("Auto", this);
    setDispatchModeAutoAct->setToolTip("Override default page behaviour");
    setDispatchModeAutoAct->setCheckable(true);


    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);

    copyAct = new QAction(QIcon(":/images/clipboard.png"), tr("&Copy to clipboard"), this);
    copyAct->setShortcuts(QKeySequence::Copy);

    findAct = new QAction(QIcon(":/images/magnifying-glass-2x.png"), tr("&Find"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setToolTip(tr("Find text in this table"));

    copyDFAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy to new dataframe"), this);
    copyDFAct->setStatusTip(tr("Copy contents to a new dataframe"));

}

void Ui_DataViewer::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(copyAct);
    editToolBar->addAction(copyDFAct);
    editToolBar->addAction(findAct);


    dispatchToolBar = addToolBar(tr("Dispatch Mode"));
    dispatchToolBar->addActions(setDispatchModeOverrideActs);
    dispatchToolBar->addAction(setDispatchModeAutoAct);

}

