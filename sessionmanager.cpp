#include "sessionmanager.h"
#include "commandsequencer.h"
#include "jobqueue.h"
#include "lazynut.h"
#include "lazynutmacro.h"
#include "lazynutjob.h"
#include "lazynutjobparam.h"
#include "answerformatter.h"
#include "answerformatterfactory.h"
#include "macroqueue.h"


#include <QtGlobal>
#include <QFinalState>
#include <QFileInfo>
#include <QBuffer>
#include <QDebug>
#include <QDomDocument>
#include <QAbstractTransition>

SessionManager* SessionManager::sessionManager = nullptr;

SessionManager *SessionManager::instance()
{
    return sessionManager ? sessionManager : (sessionManager = new SessionManager);
}

SessionManager::SessionManager()
    : lazyNutHeaderBuffer(""), lazyNutOutput(""), OOBrex("OOB secret: (\\w+)\\n")
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

void SessionManager::setupJob(LazyNutJobParam *param, QObject *sender)
{
    if (lazyNut->state() != QProcess::Running)
        return;
    LazyNutJob* job = qobject_cast<LazyNutJob*>(sender);
    bool start;
    if ((start = !job))
    {
        LazyNutMacro* macro = new LazyNutMacro(macroQueue, this);
        job = new LazyNutJob(macro); // job --> endOfMacro
        macro->setInitialState(job);
        connect(commandSequencer,SIGNAL(commandsExecuted()),macro,SIGNAL(next()));
        connect(job,SIGNAL(entered()),job,SLOT(runCommands()));
    }

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

    job->logMode = param->logMode;

    connect(job,SIGNAL(runCommands(QStringList,bool,unsigned int)),
            commandSequencer,SLOT(runCommands(QStringList,bool,unsigned int)));
    if (param->endOfJobReceiver && param->endOfJobSlot)
        connect(job,SIGNAL(exited()),param->endOfJobReceiver,param->endOfJobSlot);
    if (param->nextJobReceiver && param->nextJobSlot)
    {
        LazyNutJob* nextJob = new LazyNutJob(job->macro); // nextJob --> endOfMacro
        job->removeTransition(job->transitions().at(0));
        job->addTransition(job->macro,SIGNAL(next()),nextJob); // job --> nextJob --> endOfMacro
        connect(nextJob,SIGNAL(entered()),param->nextJobReceiver,param->nextJobSlot);
    }
    delete param;
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
//! [nextJob]
LazyNutJob *SessionManager::nextJob(QObject *sender)
{
    LazyNutJob* job = currentJob(sender);
    LazyNutJob* nextJob = qobject_cast<LazyNutJob*>(job->transitions().at(0)->targetState());
    return nextJob;
}
//! [nextJob]

//! [appendCmdListOnNextJob]
void SessionManager::appendCmdListOnNextJob(QStringList cmdList)
{
    nextJob(sender())->cmdList.append(cmdList);
}
//! [appendCmdListOnNextJob]

//! [updateRecentlyModified]
void SessionManager::updateRecentlyModified()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->cmdList = {"xml recently_modified"};
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(this, SLOT(appendCmdListOnNextJob(QStringList)));
    param->setNextJobReceiver(this, SLOT(getDescriptions()));
    setupJob(param, sender());
}
//! [updateRecentlyModified]

//! [getDescriptions]
void SessionManager::getDescriptions()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    // no cmdList, since it is set by setCmdListOnNextJob
    param->cmdFormatter = [] (QString cmd) { return cmd.prepend("xml ");};
    param->answerFormatterType = AnswerFormatterType::XML;
    param->setAnswerReceiver(this, SIGNAL(updateLazyNutObjCatalogue(QDomDocument*)));
    param->setEndOfJobReceiver(this, SIGNAL(updateDiagramScene()));
    setupJob(param, sender());
}
//! [getDescriptions]


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



//void MacroQueue::run(QStateMachine *macro)
//{
//    qDebug() << "Jobs in MacroQueue:" << queue.size();
//    macro->start();
//}

//void MacroQueue::reset()
//{
//    qDebug() << "RESET CALLED, Jobs in MacroQueue:" << queue.size();
//    while (!queue.isEmpty())
//    {
//        delete queue.dequeue();
//    }
//    if (currentJob)
//        currentJob->stop();
//    //delete currentJob;
//}

//QString MacroQueue::name()
//{
//    return "MacroQueue";
//}
