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

    QVBoxLayout *interpreterLayout = new QVBoxLayout;
    interpreterLayout->addWidget(cmdOutput);
    interpreterLayout->addWidget(inputCmdLine);
    this->setLayout(interpreterLayout);
}

