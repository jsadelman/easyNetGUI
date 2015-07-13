#include "console.h"
#include "codeeditor.h"
#include "inputcmdline.h"
#include "sessionmanager.h"

#include <QToolBar>
#include <QTextCursor>
#include <QDebug>
#include <QStatusBar>
#include <QLabel>

Console::Console(QWidget *parent)
    : EditWindow(parent)
{
    fileToolBar->removeAction(newAct);
    fileToolBar->removeAction(openAct);
    editToolBar->removeAction(cutAct);
    editToolBar->removeAction(pasteAct);

    setStyleSheet("QToolBar {background-color : gray; color : black}");
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet("background-color : black; color : white;");
    QFont qf("Courier");
    textEdit->setFont(qf);

    createStatusBar();
    statusBar()->show();


}

Console::~Console()
{

}

void Console::addText(QString txt)
{
    textEdit->moveCursor(QTextCursor::End);
    textEdit->insertPlainText(txt);
    textEdit->moveCursor(QTextCursor::End);
    currentLine = 1+textEdit->document()->blockCount();
}


void Console::createStatusBar()
{
    statusBar()->setStyleSheet("background-color : black; color : white;");

    QLabel* label = new QLabel("  >");
    inputCmdLine = new InputCmdLine(this);
    connect(inputCmdLine,SIGNAL(commandReady(QString)),
            SessionManager::instance(),SLOT(runCmd(QString)));
    connect(inputCmdLine,SIGNAL(historyKey(int)),
            this,SIGNAL(historyKey(int)));

    statusBar()->addWidget(label);
    statusBar()->addWidget(inputCmdLine,1);
    setStyleSheet( "QStatusBar::item { border: 0px}" ) ;
    inputCmdLine->setFocus();
}

void Console::showHistory(QString line)
{
    inputCmdLine->setText(line);

}

void Console::setConsoleFontSize(int size)
{
    textEdit->setFont(QFont("Courier",size));
    inputCmdLine->setFont(QFont("Courier",size));
}
