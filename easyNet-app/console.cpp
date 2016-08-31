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
    : EditWindow(parent, true)
{
//    fileToolBar->removeAction(newAct);
//    fileToolBar->removeAction(openAct);
//    editToolBar->removeAction(cutAct);
//    editToolBar->removeAction(pasteAct);

    coreDumpAct = new QAction(QIcon(":/images/log.png"), tr("Core Dump"), this);
    coreDumpAct->setStatusTip(tr("Log simulator state to file"));
    connect(coreDumpAct, SIGNAL(triggered()), this, SIGNAL(coreDumpRequested()));
    fileToolBar->addAction(coreDumpAct);

    setStyleSheet("QToolBar {background-color : gray; color : black}");
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet("background-color : black; color : white;");
    QFont qf("Consolas");
    textEdit->setFont(qf);

    createStatusBar();
    statusBar()->show();

//    connect(SessionManager::instance(), SIGNAL(lazyNutCrash()), this, SLOT(coreDump()));

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
//    connect(inputCmdLine,SIGNAL(commandReady(QString)),
//            SessionManager::instance(),SLOT(runCmd(QString)));
    connect(inputCmdLine, &InputCmdLine::commandReady, [=](QString cmd)
    {
        SessionManager::instance()->runCmd(cmd, ECHO_INTERPRETER | FROM_CONSOLE);
    });

    connect(inputCmdLine,SIGNAL(historyKey(int, QString)),
            this,SIGNAL(historyKey(int, QString)));

    statusBar()->addWidget(label);
    statusBar()->addWidget(inputCmdLine,1);
    setStyleSheet( "QStatusBar::item { border: 0px}" ) ;
    inputCmdLine->setFocus();
}

void Console::showHistory(QString line)
{
    inputCmdLine->setText(line);

}

void Console::coreDump(QString fileName)
{
    if (fileName.isEmpty())
    {
        QString logDir = SessionManager::instance()->defaultLocation("outputDir");
        QString timeStamp = QDateTime::currentDateTime().toString("yyyy.MM.dd.hh.mm.ss");
        fileName = QString("%1/core_dump.%2.log").arg(logDir).arg(timeStamp);
    }
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);

        file.write(textEdit->toPlainText().toLocal8Bit());

    file.close();
}

void Console::setConsoleFontSize(int size)
{
    int jamesMiniaturisationFactor = 1; // 0.6
    textEdit->setFont(QFont("Consolas",size * jamesMiniaturisationFactor));
    inputCmdLine->setFont(QFont("Consolas",size * jamesMiniaturisationFactor));
}
