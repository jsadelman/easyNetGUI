#include "lazynutconsole.h"
#include "cmdoutput.h"
#include "inputcmdline.h"
#include "sessionmanager.h"
#include <QVBoxLayout>

LazyNutConsole::LazyNutConsole(QWidget  *parent)
    : QGroupBox (parent)
{
    cmdOutput = new CmdOutput(this);
    cmdOutput->setReadOnly(true);
    connect(SessionManager::instance(),SIGNAL(userLazyNutOutputReady(QString)),
            cmdOutput,SLOT(displayOutput(QString)));

    inputCmdLine = new InputCmdLine(this);
    connect(inputCmdLine,SIGNAL(commandReady(QString)),
            SessionManager::instance(),SLOT(runCmd(QString)));
    connect(inputCmdLine,SIGNAL(historyKey(int)),
            this,SIGNAL(historyKey(int)));


    QVBoxLayout *interpreterLayout = new QVBoxLayout;
    interpreterLayout->addWidget(cmdOutput);
    interpreterLayout->addWidget(inputCmdLine);
    this->setLayout(interpreterLayout);
}

void LazyNutConsole::showHistory(QString line)
{
    inputCmdLine->setText(line);

}

void LazyNutConsole::setConsoleFontSize(int size)
{
    cmdOutput->setFont(QFont("Courier",size));
}

