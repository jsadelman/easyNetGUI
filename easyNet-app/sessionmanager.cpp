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
}


void SessionManager::startLazyNut(QString lazyNutBat)
{
    lazyNut->setWorkingDirectory(QFileInfo(lazyNutBat).absolutePath());
    lazyNut->setProgram(lazyNutBat);
    lazyNut->start();

    if (!lazyNut->waitForStarted())
    {
        qDebug() << lazyNut->program() << lazyNut->nativeArguments()<<  lazyNut->arguments() << lazyNut->error() << lazyNut->errorString();
        emit lazyNutNotRunning();
        qDebug("lazyNut not running");
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
    LazyNutJob* job = qobject_cast<LazyNutJob*>(sender);
    bool start;
    if ((start = !job))
    {
        LazyNutMacro* macro = new LazyNutMacro(macroQueue, this);
        connect(macro, SIGNAL(started()), this, SIGNAL(lazyNutMacroStarted()));
        connect(macro, SIGNAL(finished()), this, SIGNAL(lazyNutMacroFinished()));
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
        connect(commandSequencer,SIGNAL(answerReady(QString, QString)),
                job,SLOT(formatAnswer(QString, QString)));
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
    qDebug() << "appendCmdListOnNextJob() list size: " << cmdList.size();
    nextJob(sender())->cmdList.append(cmdList);
}

//! [appendCmdListOnNextJob]

//! [updateRecentlyModified]
void SessionManager::updateRecentlyModified()
{
    qDebug() << "updateRecentlyModified()";
    LazyNutJobParam *param = new LazyNutJobParam;
    param->cmdList = QStringList({"xml recently_modified"});
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(this, SLOT(appendCmdListOnNextJob(QStringList)));
    param->setNextJobReceiver(this, SLOT(getDescriptions()));
    setupJob(param, sender());
}
//! [updateRecentlyModified]

//! [getDescriptions]
void SessionManager::getDescriptions()
{
    qDebug() << "getDescriptions()";
    LazyNutJobParam *param = new LazyNutJobParam;
    // no cmdList, since it is set by setCmdListOnNextJob
    param->cmdFormatter = [] (QString cmd) { return cmd.prepend("xml ");};
    param->answerFormatterType = AnswerFormatterType::XML;
    param->setAnswerReceiver(this, SIGNAL(updateObjectCatalogue(QDomDocument*)));
    param->setEndOfJobReceiver(this, SIGNAL(updateDiagramScene()));
    setupJob(param, sender());
}

void SessionManager::queryRecentlyCreated()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode &= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({"xml recently_created", "clear_recently_created"});
    param->answerFormatterType = AnswerFormatterType::XML;
    param->setAnswerReceiver(this, SIGNAL(recentlyCreated(QDomDocument*)));
    param->setNextJobReceiver(this, SLOT(queryRecentlyDestroyed()));
    setupJob(param, sender());
}

void SessionManager::queryRecentlyModified()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode &= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({"xml recently_modified", "clear_recently_modified"});
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(this, SIGNAL(recentlyModified(QStringList)));
    param->setNextJobReceiver(this, SLOT(queryRecentlyCreated()));
    setupJob(param, sender());
}

void SessionManager::queryRecentlyDestroyed()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode &= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({"xml recently_destroyed", "clear_recently_destroyed"});
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(this, SIGNAL(recentlyDestroyed(QStringList)));
    param->setEndOfJobReceiver(this,SIGNAL(commandsCompleted()));
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

void SessionManager::updateObjectCatalogue()
{
    queryRecentlyModified();
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

void SessionManager::runCmd(QString cmd)
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->cmdList = QStringList({cmd});
    param->logMode |= ECHO_INTERPRETER;
    param->setNextJobReceiver(this, SLOT(updateObjectCatalogue()));
    setupJob(param);
}

void SessionManager::runCmd(QStringList cmd)
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->cmdList = cmd;
    param->logMode |= ECHO_INTERPRETER;
    param->setNextJobReceiver(this, SLOT(updateObjectCatalogue()));
    setupJob(param);
}
