#include "sessionmanager.h"
#include "commandsequencer.h"
#include "jobqueue.h"
#include "lazynut.h"

#include "lazynutjob.h"
#include "lazynutjobparam.h"
#include "answerformatter.h"
#include "answerformatterfactory.h"
#include "objectcache.h"
#include "objectcachefilter.h"
#include "easyNetMainWindow.h"


#include <QtGlobal>
#include <QFinalState>
#include <QFileInfo>
#include <QBuffer>
#include <QDebug>
#include <QDomDocument>
#include <QAbstractTransition>
#include <QThread>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QProcessEnvironment>

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
    jobQueue = new LazyNutJobQueue;

    descriptionCache = new ObjectCache(this);
    connect(this, SIGNAL(recentlyCreated(QDomDocument*)),
            descriptionCache, SLOT(create(QDomDocument*)));
    connect(this,  SIGNAL(recentlyModified(QStringList)),
            descriptionCache,  SLOT(invalidateCache(QStringList)));
    connect(this, SIGNAL(recentlyDestroyed(QStringList)),
            descriptionCache,  SLOT(destroy(QStringList)));

    ObjectCacheFilter *dfFilter = new ObjectCacheFilter(descriptionCache, this);
    dfFilter->setType("dataframe");

    dataframeCache = new ObjectCache(this);
    connect(dfFilter, SIGNAL(objectCreated(QString,QString,QDomDocument*)),
            dataframeCache, SLOT(create(QString,QString)));
    connect(dfFilter, SIGNAL(objectModified(QString)),
            dataframeCache, SLOT(invalidateCache(QString)));
    connect(dfFilter, SIGNAL(objectDestroyed(QString)),
            dataframeCache, SLOT(destroy(QString)));
}


void SessionManager::startLazyNut(QString lazyNutBat)
{
    QSettings settings("QtEasyNet", "nmConsole");
    QString easyNetHome = QDir::toNativeSeparators(settings.value("easyNetHome","../..").toString());
    QString easyNetDataHome = QDir::toNativeSeparators(settings.value("easyNetDataHome",easyNetHome).toString());
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment(); // lazyNut->processEnvironment();
    env.insert("EASYNET_HOME", easyNetHome);
    env.insert("EASYNET_DATA_HOME", easyNetDataHome);
    lazyNut->setProcessEnvironment(env);
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
    SessionManager::instance()->descriptionCache->clear();
    killLazyNut();
    lazyNut->waitForFinished();
    startLazyNut(lazyNutBat);
}



void SessionManager::submitJobs(QList<LazyNutJob *> jobs)
{
    jobQueue->tryRun(jobs);
}

LazyNutJob *SessionManager::recentlyCreatedJob()
{
    LazyNutJob *job = new LazyNutJob();
//    job->logMode |= ECHO_INTERPRETER; // debug purpose
    job->cmdList = QStringList({"xml recently_created", "clear_recently_created"});
    job->setAnswerReceiver(this, SIGNAL(recentlyCreated(QDomDocument*)), AnswerFormatterType::XML);
    return job;
}

LazyNutJob *SessionManager::recentlyModifiedJob()
{
    LazyNutJob *job = new LazyNutJob();
//    job->logMode |= ECHO_INTERPRETER; // debug purpose
    job->cmdList = QStringList({"xml recently_modified", "clear_recently_modified"});
    job->setAnswerReceiver(this, SIGNAL(recentlyModified(QStringList)), AnswerFormatterType::ListOfValues);
    return job;
}

LazyNutJob *SessionManager::recentlyDestroyedJob()
{
    LazyNutJob *job = new LazyNutJob();
//    job->logMode |= ECHO_INTERPRETER; // debug purpose
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
    jobs.last()->appendEndOfJobReceiver(this, SIGNAL(commandsCompleted()));
    return jobs;
}
//! [nextJob]






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
