#include "sessionmanager.h"
#include "commandsequencer.h"
#include "jobqueue.h"
#include "lazynut.h"
#include "lazynutmacro.h"
#include "lazynutjob.h"
#include "lazynutjobparam.h"
#include "answerformatter.h"
#include "answerformatterfactory.h"


#include <QtGlobal>
#include <QFinalState>
#include <QFileInfo>
#include <QBuffer>
#include <QDebug>
#include <QDomDocument>
#include <QAbstractTransition>

class MacroQueue;


SessionManager::SessionManager(QObject *parent)
    : QObject(parent),
      lazyNutHeaderBuffer(""), lazyNutOutput(""), OOBrex("OOB secret: (\\w+)\\n")
{
    lazyNut = new LazyNut(this);
    macroQueue = new MacroQueue;

    startCommandSequencer();
}


void SessionManager::startLazyNut(QString lazyNutBat)
{
    connect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(getOOB(QString)));
    lazyNut->setWorkingDirectory(QFileInfo(lazyNutBat).absolutePath());
    lazyNut->start(lazyNutBat);
    // TODO: here we should have a wait until timeout, since lazyNut could be on a remote server
    if (lazyNut->state() == QProcess::NotRunning)
    {
        delete lazyNut;
        emit lazyNutNotRunning();
    }
}

void SessionManager::setupJob(QObject *sender, LazyNutJobParam *param)
{
    LazyNutJob* job = qobject_cast<LazyNutJob*>(sender);
    bool start;
    if ((start = !job))
    {
        LazyNutMacro* macro = new LazyNutMacro(this);
        job = new LazyNutJob(macro); // job --> endOfMacro
        macro->setInitialState(job);
        connect(commandSequencer,SIGNAL(commandsExecuted()),macro,SIGNAL(next()));
        connect(job,SIGNAL(entered()),job,SLOT(runCommands()));
    }
    job->jobOrigin = param->jobOrigin;
    if (job->cmdList.isEmpty())
        job->setCmdList(param->cmdList);
    job->setCmdFormatter(param->cmdFormatter);
    AnswerFormatterFactory* factory = AnswerFormatterFactory::instance();
    AnswerFormatter *af = factory->newAnswerFormatter(param->answerFormatterType,
                                                      param->answerReceiver,
                                                      param->answerSlot);
    if (af)
    {
        connect(commandSequencer,SIGNAL(answerReady(QString)),
                af,SLOT(formatAnswer(QString)));
        job->setAnswerFormatter(af);
    }

    connect(job,SIGNAL(runCommands(QStringList,JobOrigin)),
            commandSequencer,SLOT(runCommands(QStringList,JobOrigin)));
    if (param->finalReceiver && param->finalSlot)
        connect(job,SIGNAL(exited()),param->finalReceiver,param->finalSlot);
    if (param->nextJobReceiver && param->nextJobSlot)
    {
        LazyNutJob* nextJob = new LazyNutJob(job->macro); // nextJob --> endOfMacro
        job->removeTransition(job->transitions().at(0));
        job->addTransition(job->macro,SIGNAL(next()),nextJob); // job --> nextJob --> endOfMacro
        connect(nextJob,SIGNAL(entered()),param->nextJobReceiver,param->nextJobSlot);
    }

    if (start)
        macroQueue->tryRun(job->macro);
    else
        job->runCommands();
}


LazyNutJob *SessionManager::currentJob(QObject *sender)
{
    LazyNutJob* job = qobject_cast<LazyNutJob*> (sender);
    if (job)
        return job;
    AnswerFormatter *af = qobject_cast<AnswerFormatter*> (sender);
    if (af)
    {
        LazyNutJob* job = qobject_cast<LazyNutJob*> (af->parent());
        return job;
    }
    return nullptr;
}

LazyNutJob *SessionManager::nextJob(QObject *sender)
{
    LazyNutJob* job = currentJob(sender);
    LazyNutJob* nextJob = qobject_cast<LazyNutJob*>(job->transitions().at(0)->targetState());
    return nextJob;
}


void SessionManager::getOOB(const QString &lazyNutOutput)
{
    lazyNutHeaderBuffer.append(lazyNutOutput);
    if (lazyNutHeaderBuffer.contains(OOBrex))
    {
        OOBsecret = OOBrex.cap(1);
        qDebug() << OOBsecret;
        lazyNutHeaderBuffer.clear();
        disconnect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(getOOB(QString)));
    }
}

void SessionManager::startCommandSequencer()
{
    commandSequencer = new CommandSequencer(lazyNut, this);
    connect(commandSequencer,SIGNAL(userLazyNutOutputReady(QString)),
            this,SIGNAL(userLazyNutOutputReady(QString)));
}


void SessionManager::killLazyNut()
{
    lazyNut->kill();
}


bool SessionManager::getStatus()
{
    return commandSequencer->getStatus();
}

void SessionManager::pause()
{
    macroQueue->pause();
//    commandSequencer->pause();
    emit isPaused(macroQueue->isPaused());
}

void SessionManager::stop()
{
    macroQueue->stop();
//    commandSequencer->stop();
}



void SessionManager::macroStarted()
{
    qDebug() << "Macro started.";
}

void SessionManager::macroEnded()
{
    qDebug() << "Macro ended.";
    macroQueue->freeToRun();
}



void MacroQueue::run(QStateMachine *macro)
{
    qDebug() << "Jobs in MacroQueue:" << queue.size();
    macro->start();
}

void MacroQueue::reset()
{
    qDebug() << "RESET CALLED, Jobs in MacroQueue:" << queue.size();
    while (!queue.isEmpty())
    {
        delete queue.dequeue();
    }
    if (currentJob)
        currentJob->stop();
    //delete currentJob;
}

QString MacroQueue::name()
{
    return "MacroQueue";
}
