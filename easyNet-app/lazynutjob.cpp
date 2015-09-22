#include "lazynutjob.h"
#include "sessionmanager.h"
#include "commandsequencer.h"
#include "answerformatter.h"
#include "answerformatterfactory.h"
#include "jobqueue.h"

LazyNutJob::LazyNutJob()
    : m_active(false),
      answerFormatter(nullptr),
      logMode(0)
{
    cmdList.clear();
    connect(this,SIGNAL(runCommands(QStringList,bool,unsigned int)),
            SessionManager::instance()->commandSequencer,SLOT(runCommands(QStringList,bool,unsigned int)));
    connect(SessionManager::instance()->commandSequencer,SIGNAL(commandsExecuted()),
            this,SLOT(finish()));

}

LazyNutJob::~LazyNutJob()
{
}

void LazyNutJob::setAnswerReceiver(QObject *receiver, const char *slot, AnswerFormatterType answerFormatterType)
{
    AnswerFormatterFactory* factory = AnswerFormatterFactory::instance();
    answerFormatter = factory->newAnswerFormatter(answerFormatterType,
                                                      receiver,
                                                      slot);
    if (answerFormatter)
    {
        answerFormatter->setParent(this);
        connect(SessionManager::instance()->commandSequencer,SIGNAL(answerReady(QString, QString)),
                this,SLOT(formatAnswer(QString, QString)));
    }
}

void LazyNutJob::setErrorReceiver(QObject *receiver, const char *slot)
{
    connect(SessionManager::instance()->commandSequencer,SIGNAL(cmdError(QString, QStringList)),
            this, SLOT(sendCmdError(QString, QStringList)));
    connect(this, SIGNAL(cmdError(QString, QStringList)),
            receiver, slot);
}

void LazyNutJob::setEndOfJobReceiver(QObject *receiver, const char *slot)
{
    connect(this, SIGNAL(finished()), receiver, slot);
}

void LazyNutJob::run()
{
    setActive(true);
    emit runCommands(cmdList, answerFormatter, logMode);
}

void LazyNutJob::finish()
{
    if (active())
    {
        emit finished();
        SessionManager::instance()->jobQueue->freeToRun();
        deleteLater();
    }
}

void LazyNutJob::formatAnswer(QString answer, QString cmd)
{
    if (active() && answerFormatter)
        answerFormatter->formatAnswer(answer, cmd);
}

void LazyNutJob::sendCmdError(QString cmd, QStringList errorList)
{
    if (active())
        emit cmdError(cmd, errorList);
}

