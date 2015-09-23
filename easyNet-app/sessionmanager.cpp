#include "sessionmanager.h"
#include "commandsequencer.h"
#include "jobqueue_DEPRECATED.h"
#include "jobqueue.h"
#include "lazynut.h"
#include "lazynutmacro.h"
#include "lazynutjob_DEPRECATED.h"
#include "lazynutjob.h"
#include "lazynutjobparam.h"
#include "answerformatter.h"
#include "answerformatterfactory.h"
#include "macroqueue.h"
#include "objectcatalogue.h"


#include <QtGlobal>
#include <QFinalState>
#include <QFileInfo>
#include <QBuffer>
#include <QDebug>
#include <QDomDocument>
#include <QAbstractTransition>
#include <QThread>
#include <QMessageBox>

SessionManager* SessionManager::sessionManager = nullptr;

SessionManager *SessionManager::instance()
{
    return sessionManager ? sessionManager : (sessionManager = new SessionManager);
}

SessionManager::SessionManager()
    : lazyNutHeaderBuffer(""), lazyNutOutput(""), OOBrex("OOB secret: (\\w+)(?=\\r\\n)")
{
    lazyNut = new LazyNut(this);
    connect(lazyNut, SIGNAL(started()), this, SLOT(startCommandSequencer()));
    connect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(getOOB(QString)));
    macroQueue = new MacroQueue;
    jobQueue = new LazyNutJobQueue;
}


void SessionManager::startLazyNut(QString lazyNutBat)
{
    lazyNut->setWorkingDirectory(QFileInfo(lazyNutBat).absolutePath());
    lazyNut->setProgram(lazyNutBat);
    connect(lazyNut, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(sendLazyNutCrash(int,QProcess::ExitStatus)));

    lazyNut->start();

    if (!lazyNut->waitForStarted())
    {
        qDebug() << lazyNut->program() <<
            #ifdef WIN32
                    lazyNut->nativeArguments()<<
            #endif
                    lazyNut->arguments() << lazyNut->error() << lazyNut->errorString();
        emit lazyNutNotRunning();
        qDebug("lazyNut not running");
    }
}

void SessionManager::sendLazyNutCrash(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0 || exitStatus !=QProcess::NormalExit)
    {
//        qDebug() << "exit status " << exitCode << exitStatus;
     //   emit lazyNutCrash();
    }
}


void SessionManager::restartLazyNut(QString lazyNutBat)
{
//    ObjectCatalogue::instance()->removeRows(0, ObjectCatalogue::instance()->rowCount());
    ObjectCatalogue::instance()->clear();

    killLazyNut();
    lazyNut->waitForFinished();
    startLazyNut(lazyNutBat);
}

void SessionManager::setupJob(LazyNutJobParam *param, QObject *sender)
{
    if (lazyNut->state() != QProcess::Running)
        return;
    LazyNutJob_DEPRECATED* job = qobject_cast<LazyNutJob_DEPRECATED*>(sender);
    bool start;
    if ((start = !job))
    {
        LazyNutMacro* macro = new LazyNutMacro(macroQueue, this);
        connect(macro, SIGNAL(started()), this, SIGNAL(lazyNutMacroStarted()));
        connect(macro, SIGNAL(finished()), this, SIGNAL(lazyNutMacroFinished()));
        job = new LazyNutJob_DEPRECATED(macro); // job --> endOfMacro
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
        connect(commandSequencer,SIGNAL(answerReady(QString, QString)),
                job,SLOT(formatAnswer(QString, QString)));
        job->setAnswerFormatter(af);

    }

    job->logMode = param->logMode;

    connect(job,SIGNAL(runCommands(QStringList,bool,unsigned int)),
            commandSequencer,SLOT(runCommands(QStringList,bool,unsigned int)));

    if (param->errorReceiver && param->errorSlot)
    {
        connect(commandSequencer,SIGNAL(cmdError(QString, QStringList)),
                job, SIGNAL(cmdError(QString, QStringList)));
        connect(job, SIGNAL(cmdError(QString, QStringList)),
            param->errorReceiver, param->errorSlot);
    }

    if (param->endOfJobReceiver && param->endOfJobSlot)
        connect(job,SIGNAL(exited()),param->endOfJobReceiver, param->endOfJobSlot);
    if (param->nextJobReceiver && param->nextJobSlot)
    {
        LazyNutJob_DEPRECATED* nextJob = new LazyNutJob_DEPRECATED(job->macro); // nextJob --> endOfMacro
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

void SessionManager::setupNoOp(QObject *sender)
{
    setupJob(new LazyNutJobParam, sender);
}


LazyNutJob_DEPRECATED *SessionManager::currentJob(QObject *sender)
{
    LazyNutJob_DEPRECATED* job = qobject_cast<LazyNutJob_DEPRECATED*> (sender);
    if (job)
        return job;
    AnswerFormatter *af = qobject_cast<AnswerFormatter*> (sender);
    if (af)
    {
        LazyNutJob_DEPRECATED* job = qobject_cast<LazyNutJob_DEPRECATED*> (af->parent());
        return job;
    }
    return nullptr;
}
//! [nextJob]
LazyNutJob_DEPRECATED *SessionManager::nextJob(QObject *sender)
{
    LazyNutJob_DEPRECATED* job = currentJob(sender);
    LazyNutJob_DEPRECATED* nextJob = qobject_cast<LazyNutJob_DEPRECATED*>(job->transitions().at(0)->targetState());
    return nextJob;
}

void SessionManager::submitJobs(QList<LazyNutJob *> jobs)
{
    jobQueue->tryRun(jobs);
}

LazyNutJob *SessionManager::recentlyCreatedJob()
{
    LazyNutJob *job = new LazyNutJob();
    job->logMode |= ECHO_INTERPRETER; // debug purpose
    job->cmdList = QStringList({"xml recently_created", "clear_recently_created"});
    job->setAnswerReceiver(this, SIGNAL(recentlyCreated(QDomDocument*)), AnswerFormatterType::XML);
    return job;
}

LazyNutJob *SessionManager::recentlyModifiedJob()
{
    LazyNutJob *job = new LazyNutJob();
    job->logMode |= ECHO_INTERPRETER; // debug purpose
    job->cmdList = QStringList({"xml recently_modified", "clear_recently_modified"});
    job->setAnswerReceiver(this, SIGNAL(recentlyModified(QStringList)), AnswerFormatterType::ListOfValues);
    return job;
}

LazyNutJob *SessionManager::recentlyDestroyedJob()
{
    LazyNutJob *job = new LazyNutJob();
    job->logMode |= ECHO_INTERPRETER; // debug purpose
    job->cmdList = QStringList({"xml recently_destroyed", "clear_recently_destroyed"});
    job->setAnswerReceiver(this, SIGNAL(recentlyDestroyed(QStringList)), AnswerFormatterType::ListOfValues);
    return job;
}

QList<LazyNutJob *> SessionManager::updateObjectCatalogueJobs()
{
    QList<LazyNutJob *> jobs =  QList<LazyNutJob *> ()
                                << recentlyModifiedJob()
                                << recentlyCreatedJob()
                                << recentlyDestroyedJob();
    jobs.last()->setEndOfJobReceiver(this, SIGNAL(commandsCompleted()));
    return jobs;
}
//! [nextJob]

//! [appendCmdListOnNextJob]
void SessionManager::appendCmdListOnNextJob(QStringList cmdList)
{
    qDebug() << "appendCmdListOnNextJob() list size: " << cmdList.size();
    nextJob(sender())->cmdList.append(cmdList);
}

//! [appendCmdListOnNextJob]





void SessionManager::getOOB(const QString &lazyNutOutput)
{
    lazyNutHeaderBuffer.append(lazyNutOutput);
    if (lazyNutHeaderBuffer.contains(OOBrex))
    {
        OOBsecret = OOBrex.cap(1);
        qDebug() << OOBsecret;
        lazyNutHeaderBuffer.clear();
        disconnect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(getOOB(QString)));
        emit lazyNutStarted();
    }
}

void SessionManager::startCommandSequencer()
{
    commandSequencer = new CommandSequencer(lazyNut, this);

    connect(commandSequencer,SIGNAL(userLazyNutOutputReady(QString)),
            this,SIGNAL(userLazyNutOutputReady(QString)));
    connect(commandSequencer, SIGNAL(isReady(bool)),
            this, SIGNAL(isReady(bool)));
    connect(commandSequencer, SIGNAL(cmdError(QString,QStringList)),
            this, SIGNAL(cmdError(QString,QStringList)));
    connect(commandSequencer, SIGNAL(commandExecuted(QString,QString)),
            this, SIGNAL(commandExecuted(QString,QString)));
    connect(commandSequencer, SIGNAL(commandsInJob(int)),
            this, SIGNAL(commandsInJob(int)));
    connect(commandSequencer, SIGNAL(commandSent(QString)),
            this, SIGNAL(commandSent(QString)));
    connect(commandSequencer, SIGNAL(logCommand(QString)),
            this, SIGNAL(logCommand(QString)));



    emit isReady(commandSequencer->getStatus());
}

void SessionManager::lazyNutProcessError(int error)
{
    qDebug() << "lazyNut process cannot start. QProcess::ProcessError" << error;
}


void SessionManager::killLazyNut()
{
    lazyNut->kill();
}



bool SessionManager::isReady()
{
    return commandSequencer->getStatus();
}

bool SessionManager::isOn()
{
    return commandSequencer->isOn();
}

void SessionManager::pause()
{
    macroQueue->pause();
//    commandSequencer->pause();
    emit isPaused(macroQueue->isPaused());
}



void SessionManager::runCmd(QString cmd)
{
    runCmd(QStringList({cmd}));
}

void SessionManager::runCmd(QStringList cmd)
{
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = cmd;
    job->logMode |= ECHO_INTERPRETER;
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << updateObjectCatalogueJobs();
    submitJobs(jobs);
}
