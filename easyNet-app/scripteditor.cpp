#include "scripteditor.h"
#include "codeeditor.h"
#include <QAction>
#include <QToolBar>
#include <QTextCursor>
#include <QDebug>
#include <QShortcut>


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

    stopScriptAct = new QAction("STOP",this);
//    connect(stopScriptAct,SIGNAL(triggered()),SessionManager::instance(),SLOT(stop()));
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
    emit runCmdRequested(textEdit->getAllText());
}

void ScriptEditor::runSelection()
{
    QTextCursor *cursor = new QTextCursor(textEdit->document());
    if (cursor->selectedText().trimmed().isEmpty())
    {
//        qDebug() << "runSelection -- nothing selected";
//        qDebug() << "runSelection -- cursorPos" << cursor->position();
        cursor->movePosition(QTextCursor::StartOfLine);
//        qDebug() << "runSelection -- cursorPos" << cursor->position();
        cursor->movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
//        qDebug() << "runSelection -- cursorPos" << cursor->position();
//        qDebug() << "runSelection -- selected:" << textEdit->getCurrentLine();
        emit runCmdRequested(QStringList(textEdit->getCurrentLine()));
    }
    else
        emit runCmdRequested(textEdit->getSelectedText());
}


