#include "resultswindow_if.h"
#include "lazynutjob.h"
#include "enumclasses.h"
#include "sessionmanager.h"

#include <QDomDocument>
#include <QAction>
#include <QMenu>
#include <QSignalMapper>
#include <QDebug>
#include <QToolBar>
#include <QMenuBar>
#include <QDockWidget>
#include <QScrollArea>

Q_DECLARE_METATYPE(QDomDocument*)

ResultsWindow_If::ResultsWindow_If(QWidget *parent)
   :  dispatchModeOverride(-1), dispatchModeAuto(true), QMainWindow(parent)
{
    dispatchModeName.insert(Dispatch_New, "New Page");
    dispatchModeName.insert(Dispatch_Overwrite, "Overwrite");
    dispatchModeName.insert(Dispatch_Append, "Append");

    dispatchModeIconName.insert(Dispatch_New, ":/images/tab_new.png");
    dispatchModeIconName.insert(Dispatch_Overwrite, ":/images/overwrite.png");
    dispatchModeIconName.insert(Dispatch_Append, ":/images/append.png");

    dispatchModeFST.insert(qMakePair(Dispatch_New,Dispatch_New), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_New,Dispatch_Overwrite), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_New,Dispatch_Append), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_Overwrite,Dispatch_New), Dispatch_Overwrite);
    dispatchModeFST.insert(qMakePair(Dispatch_Overwrite,Dispatch_Overwrite), Dispatch_Overwrite);
    dispatchModeFST.insert(qMakePair(Dispatch_Overwrite,Dispatch_Append), Dispatch_Overwrite);
    dispatchModeFST.insert(qMakePair(Dispatch_Append,Dispatch_New), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_Append,Dispatch_Overwrite), Dispatch_New);
    dispatchModeFST.insert(qMakePair(Dispatch_Append,Dispatch_Append), Dispatch_Append);

    // info
    infoDock = new QDockWidget("Info",this);
    infoScroll = new QScrollArea(this);
    infoScroll->setWidgetResizable(true);
    infoDock->setWidget(infoScroll);
    addDockWidget(Qt::BottomDockWidgetArea, infoDock);
    infoDock->setFeatures(QDockWidget::DockWidgetClosable);

    infoVisible = false;
    infoDock->close();



}

ResultsWindow_If::~ResultsWindow_If()
{
}

void ResultsWindow_If::dispatch()
{
    QVariant info = SessionManager::instance()->getDataFromJob(sender(), "trialRunInfo");
    if (info.canConvert<QDomDocument *>())
    {
        dispatch_Impl(info.value<QDomDocument*>());
    }
    else
    {
        qDebug() << "ERROR: ResultsWindow_If::dispatch cannot convert trialRunInfo to QDomDocument";
    }
}


void ResultsWindow_If::setDispatchModeOverride(int mode)
{
    dispatchModeOverride = mode;
}

void ResultsWindow_If::setDispatchModeAuto(bool isAuto)
{
    dispatchModeAuto = isAuto;
//    setDispatchModeOverrideActGroup->setDisabled(isAuto);
    setDispatchModeOverrideActGroup->setVisible(!isAuto);
}

void ResultsWindow_If::createActions()
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
    connect(setDispatchModeOverrideMapper, SIGNAL(mapped(int)),
            this, SLOT(setDispatchModeOverride(int)));

    setDispatchModeAutoAct = new QAction("Auto", this);
    setDispatchModeAutoAct->setToolTip("Override default tabs behaviour");
    setDispatchModeAutoAct->setCheckable(true);
//    setDispatchModeOverrideActGroup->addAction(setDispatchModeAutoAct);
    connect(setDispatchModeAutoAct, SIGNAL(triggered(bool)),
            this, SLOT(setDispatchModeAuto(bool)));

    setDispatchModeAuto(true);
    setDispatchModeAutoAct->setChecked(true);


    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    copyAct = new QAction(QIcon(":/images/clipboard.png"), tr("&Copy to clipboard"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    infoAct = infoDock->toggleViewAction();
    infoAct->setIcon(QIcon(":/images/Information-icon.png"));
    infoAct->setText(tr("&Info"));
    infoAct->setToolTip(tr("Show/hide trial run info"));
    connect(infoAct, SIGNAL(triggered(bool)), this, SLOT(setInfoVisible(bool)));

}

//void ResultsWindow_If::createMenus()
//{
//    fileMenu = menuBar()->addMenu(tr("&File"));
//    fileMenu->addAction(openAct);
//    fileMenu->addAction(saveAct);

//    editMenu = menuBar()->addMenu(tr("&Edit"));
//    editMenu->addAction(copyAct);

//    settingsMenu = menuBar()->addMenu(tr("&Settings"));
//    setSingleTrialDispatchModeMenu = settingsMenu->addMenu(tr("Dispatch single trial"));
//    for (int mode = 0; mode < MAX_DISPATCH_MODE; ++mode)
//        setSingleTrialDispatchModeMenu->addAction(setSingleTrialDispatchModeActs.at(mode));

//    setTrialListDispatchModeMenu = settingsMenu->addMenu(tr("Dispatch trial list"));
//    for (int mode = 0; mode < MAX_DISPATCH_MODE; ++mode)
//        setTrialListDispatchModeMenu->addAction(setTrialListDispatchModeActs.at(mode));

//    dispatchModeOverrideMenu = settingsMenu->addMenu(tr("Override dispatch mode"));
//    dispatchModeOverrideMenu->addAction(setDispatchModeAutoAct);
//    for (int mode = 0; mode < MAX_DISPATCH_MODE; ++mode)
//        dispatchModeOverrideMenu->addAction(setDispatchModeOverrideActs.at(mode));
//}

void ResultsWindow_If::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(copyAct);

    dispatchToolBar = addToolBar(tr("Dispatch Mode"));
    dispatchToolBar->addActions(setDispatchModeOverrideActs);
    dispatchToolBar->addAction(setDispatchModeAutoAct);

    infoToolBar = addToolBar(tr("Info"));
    infoToolBar->addAction(infoAct);
}

