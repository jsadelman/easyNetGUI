#include "ui_dataviewer.h"
#include "enumclasses.h"
#include "sessionmanager.h"
#include "xmlaccessor.h"
#include "objectcachefilter.h"
#include "objectupdater.h"



#include <QAction>
#include <QSignalMapper>
#include <QDebug>
#include <QToolBar>

Ui_DataViewer::Ui_DataViewer(bool usePrettyNames, QWidget *parent)
    : QMainWindow(parent), m_usePrettyNames(usePrettyNames)
{
}

Ui_DataViewer::~Ui_DataViewer()
{
}

void Ui_DataViewer::setupUi(DataViewer *dataViewer)
{
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
    dataViewer->setDispatchModeAuto(true);
    setDispatchModeAutoAct->setChecked(true);
    connect(openAct, SIGNAL(triggered()), dataViewer, SLOT(open()));
    connect(saveAct, SIGNAL(triggered()), dataViewer, SLOT(save()));
    connect(copyAct, SIGNAL(triggered()), dataViewer, SLOT(copy()));
}

void Ui_DataViewer::createActions()
{
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
//    setDispatchModeAutoAct->setToolTip("Override default tabs behaviour");
    setDispatchModeAutoAct->setCheckable(true);

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);

    copyAct = new QAction(QIcon(":/images/clipboard.png"), tr("&Copy to clipboard"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
}

void Ui_DataViewer::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(copyAct);

    dispatchToolBar = addToolBar(tr("Dispatch Mode"));
    dispatchToolBar->addActions(setDispatchModeOverrideActs);
    dispatchToolBar->addAction(setDispatchModeAutoAct);

}

