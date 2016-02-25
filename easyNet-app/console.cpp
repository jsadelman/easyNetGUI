#include "console.h"
#include "codeeditor.h"
#include "inputcmdline.h"
#include "sessionmanager.h"

#include <QToolBar>
#include <QTextCursor>
#include <QDebug>
#include <QStatusBar>
#include <QLabel>
#include <QSettings>
#include <QAction>
#include <QDateTime>

Console::Console(QWidget *parent)
    : EditWindow(parent)
{
    fileToolBar->removeAction(newAct);
    fileToolBar->removeAction(openAct);
    editToolBar->removeAction(cutAct);
    editToolBar->removeAction(pasteAct);

    coreDumpAct = new QAction(QIcon(":/images/Radioactive-icon.png"), tr("Core Dump"), this);
    coreDumpAct->setStatusTip(tr("Save the content of the console to file"));
    connect(coreDumpAct, SIGNAL(triggered()), this, SLOT(coreDump()));
    fileToolBar->addAction(coreDumpAct);

    setStyleSheet("QToolBar {background-color : gray; color : black}");
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet("background-color : black; color : white;");
    QFont qf("Courier");
    textEdit->setFont(qf);

    createStatusBar();
    statusBar()->show();

    connect(SessionManager::instance(), SIGNAL(lazyNutCrash()), this, SLOT(coreDump()));

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

void Console::coreDump()
{
    QSettings settings("easyNet", "GUI");
    QString logDir = SessionManager::instance()->defaultLocation("outputDir");
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy.MM.dd.hh.mm.ss");
    QString fileName = QString("%1/core_dump.%2.log").arg(logDir).arg(timeStamp);
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);

        file.write(textEdit->toPlainText().toLocal8Bit());

    file.close();
}

void Console::setConsoleFontSize(int size)
{
    textEdit->setFont(QFont("Courier",size));
    inputCmdLine->setFont(QFont("Courier",size));
}
