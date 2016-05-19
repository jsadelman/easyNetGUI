#include "lazynutjob.h"
#include "sessionmanager.h"
#include "commandsequencer.h"
#include "answerformatter.h"
#include "answerformatterfactory.h"
#include "jobqueue.h"

#include <QDebug>

LazyNutJob::LazyNutJob()
    : m_active(false),
      answerFormatter(nullptr),
      logMode(0),
      cmdList({}),
      errorReceiverList({}),
      endOfJobReceiverList({}),
      data()
{
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
}


void LazyNutJob::run()
{
    if (answerFormatter)
    {
        answerFormatter->setParent(this);
        connect(SessionManager::instance()->commandSequencer,SIGNAL(answerReady(QString, QString)),
                this,SLOT(formatAnswer(QString, QString)));
    }
    if (!errorReceiverList.isEmpty())
    {
        connect(SessionManager::instance()->commandSequencer,SIGNAL(cmdError(QString, QString)),
                this, SLOT(sendCmdError(QString, QString)));
        for (int i = 0; i < errorReceiverList.length(); ++i)
        {
            connect(this, SIGNAL(cmdError(QString, QString)),
                    errorReceiverList.at(i), errorSlotList.at(i));
        }
    }
    if (!endOfJobReceiverList.isEmpty())
    {
        for (int i = 0; i < endOfJobReceiverList.length(); ++i)
        {
            connect(this, SIGNAL(finished()),
                    endOfJobReceiverList.at(i), endOfJobSlotList.at(i));
        }
    }
//    connect(this,SIGNAL(runCommands(QStringList,bool,unsigned int)),
//            SessionManager::instance()->commandSequencer,SLOT(runCommands(QStringList,bool,unsigned int)));
    connect(SessionManager::instance()->commandSequencer,SIGNAL(jobExecuted()),
            this,SLOT(finish()));
    setActive(true);
//    qDebug() << "run" << this << cmdList;
    SessionManager::instance()->commandSequencer->runCommands(cmdList, answerFormatter, logMode);
//    emit runCommands(cmdList, answerFormatter, logMode);
}

void LazyNutJob::finish()
{
    if (active())
    {
//        qDebug() << "LazyNutJob::finish()" << this <<  sender()->metaObject()->className() << sender();
        setActive(false);
        if (answerFormatter)
            delete answerFormatter;
        // disconnect
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

void LazyNutJob::sendCmdError(QString cmd, QString error)
{
    if (active())
        emit cmdError(cmd, error);
}

