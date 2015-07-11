#include "scripteditor.h"
#include "codeeditor.h"
#include <QAction>
#include <QToolBar>

ScriptEditor::ScriptEditor(QWidget *parent)
    : EditWindow(parent)
{
    runAct = new QAction(QIcon(":/images/media-play-3x.png"), tr("&Run"), this);
//    runAct->setShortcuts(QKeySequence::New);
    runAct->setStatusTip(tr("Run script"));
    connect(runAct, SIGNAL(triggered()), this, SLOT(runScript()));

    runSelectionAct = new QAction(QIcon(":/images/reload-2x.png"),tr("Run se&lection"), this);
    runSelectionAct->setStatusTip(tr("Run selected text"));
    connect(runSelectionAct,SIGNAL(triggered()),this, SLOT(runSelection()));

    stopAct = new QAction("STOP",this);
//    connect(stopAct,SIGNAL(triggered()),SessionManager::instance(),SLOT(stop()));
    pauseAct = new QAction("PAUSE",this);
//    connect(pauseAct,SIGNAL(triggered()),SessionManager::instance(),SLOT(pause()));
//    connect(SessionManager::instance(),SIGNAL(isPaused(bool)),this,SLOT(showPauseState(bool)));

    runToolBar = addToolBar(tr("Run"));
    runToolBar->addAction(runAct);
    runToolBar->addAction(runSelectionAct);
//    runToolBar->addAction(pasteAct);

}

ScriptEditor::~ScriptEditor()
{

}

void ScriptEditor::runScript()
{
    emit runCmdAndUpdate(textEdit->getAllText());
}

void ScriptEditor::runSelection()
{
    emit runCmdAndUpdate(textEdit->getSelectedText());
}


