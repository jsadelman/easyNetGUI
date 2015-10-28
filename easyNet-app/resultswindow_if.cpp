#include "resultswindow_if.h"
#include "lazynutjob.h"

#include <QDomDocument>
#include <QAction>
#include <QMenu>
#include <QSignalMapper>
#include <QDebug>
#include <QToolBar>
#include <QMenuBar>

Q_DECLARE_METATYPE(QDomDocument*)

ResultsWindow_If::ResultsWindow_If(QWidget *parent)
   : QMainWindow(parent)
{
    dispatchModeName.insert(New, "New");
    dispatchModeName.insert(Overwrite, "Overwrite");
    dispatchModeName.insert(Append, "Append");

    dispatchFST.insert(qMakePair(Overwrite, Overwrite), Overwrite);
    dispatchFST.insert(qMakePair(Overwrite, Append), Overwrite);
    dispatchFST.insert(qMakePair(Overwrite, New), Overwrite);
    dispatchFST.insert(qMakePair(Append, Overwrite), New);
    dispatchFST.insert(qMakePair(Append, Append), Append);
    dispatchFST.insert(qMakePair(Append, New), New);
    dispatchFST.insert(qMakePair(New, Overwrite), New);
    dispatchFST.insert(qMakePair(New, Append), New);
    dispatchFST.insert(qMakePair(New, New), New);

}

ResultsWindow_If::~ResultsWindow_If()
{
}

void ResultsWindow_If::dispatch()
{
    LazyNutJob *job = qobject_cast<LazyNutJob *>(sender());
    if (!job)
    {
        qDebug() << "ERROR: ResultsWindow_If::dispatch cannot extract LazyNutJob from sender";
        return;
    }
    QMap<QString, QVariant> data = job->data.toMap();
    if (!data.contains("trialRunInfo"))
    {
        qDebug() << "ERROR: ResultsWindow_If::dispatch LazyNutJob->data does not contain trialRunInfo entry";
        return;
    }
    if (!data.value("trialRunInfo").canConvert<QDomDocument *>())
    {
        qDebug() << "ERROR: ResultsWindow_If::dispatch cannot convert trialRunInfo to QDomDocument";
        return;
    }
    dispatch_Impl(data.value("trialRunInfo").value<QDomDocument*>());
}

void ResultsWindow_If::createActions()
{
    // single trial
    setSignleTrialDispatchModeMapper = new QSignalMapper(this);
    setSignleTrialDispatchModeActGrouop = new QActionGroup(this);
    for (int mode = 0; mode < MAX_DISPATCH_MODE; ++mode)
    {
        setSingleTrialDispatchModeActs.insert(mode, new QAction(dispatchModeName.value(mode), this));
        setSingleTrialDispatchModeActs.at(mode)->setCheckable(true);
        setSignleTrialDispatchModeMapper->setMapping(setSingleTrialDispatchModeActs.at(mode), mode);
        connect(setSingleTrialDispatchModeActs.at(mode), SIGNAL(triggered()),
                setSignleTrialDispatchModeMapper, SLOT(map()));
        setSignleTrialDispatchModeActGrouop->addAction(setSingleTrialDispatchModeActs.at(mode));
    }
    connect(setSignleTrialDispatchModeMapper, SIGNAL(mapped(int)),
            this, SLOT(setSingleTrialMode(int)));

    // trial list
    setTrialListDispatchModeMapper = new QSignalMapper(this);
    setTrialListDispatchModeActGrouop = new QActionGroup(this);
    for (int mode = 0; mode < MAX_DISPATCH_MODE; ++mode)
    {
        setTrialListDispatchModeActs.insert(mode, new QAction(dispatchModeName.value(mode), this));
        setTrialListDispatchModeActs.at(mode)->setCheckable(true);
        setTrialListDispatchModeMapper->setMapping(setTrialListDispatchModeActs.at(mode), mode);
        connect(setTrialListDispatchModeActs.at(mode), SIGNAL(triggered()),
                setTrialListDispatchModeMapper, SLOT(map()));
        setTrialListDispatchModeActGrouop->addAction(setTrialListDispatchModeActs.at(mode));
    }
    connect(setTrialListDispatchModeMapper, SIGNAL(mapped(int)),
            this, SLOT(setTrialListMode(int)));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    copyAct = new QAction(QIcon(":/images/clipboard.png"), tr("&Copy to clipboard"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));
}

void ResultsWindow_If::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(copyAct);

    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    setSingleTrialDispatchModeMenu = settingsMenu->addMenu(tr("Single trial run"));
    for (int mode = 0; mode < MAX_DISPATCH_MODE; ++mode)
        setSingleTrialDispatchModeMenu->addAction(setSingleTrialDispatchModeActs.at(mode));

    setTrialListDispatchModeMenu = settingsMenu->addMenu(tr("Trial list run"));
    for (int mode = 0; mode < MAX_DISPATCH_MODE; ++mode)
        setTrialListDispatchModeMenu->addAction(setTrialListDispatchModeActs.at(mode));
}

void ResultsWindow_If::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(copyAct);
}

