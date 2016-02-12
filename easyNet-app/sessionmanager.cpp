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
#include "objectnamevalidator.h"



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
    : lazyNutHeaderBuffer(""), lazyNutOutput(""), OOBrex("OOB secret: (\\w+)(?=\\r\\n)"),
      m_plotFlags(), m_suspendingObservers(false)
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

    dataframeCache = new ObjectCache(this);
    connect(this, SIGNAL(recentlyCreated(QDomDocument*)),
            dataframeCache, SLOT(create(QDomDocument*)));
    connect(this,  SIGNAL(recentlyModified(QStringList)),
            dataframeCache,  SLOT(invalidateCache(QStringList)));
    connect(this, SIGNAL(recentlyDestroyed(QStringList)),
            dataframeCache,  SLOT(destroy(QStringList)));

    validator = new ObjectNameValidator(this);
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

void SessionManager::setPrettyName(QString name, QString prettyName)
{
    if (!descriptionCache->exists(name))
        return;
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList << QString("%1 set_pretty_name %2").arg(name).arg(prettyName);
    QList<LazyNutJob *> jobs =  QList<LazyNutJob *> ()
                                << job
                                << recentlyModifiedJob();
    submitJobs(jobs);
}

void SessionManager::destroyObject(QString name)
{
    if (descriptionCache->exists(name))
    {
        LazyNutJob *job = new LazyNutJob;
        job->logMode |= ECHO_INTERPRETER;
        job->cmdList << QString("destroy %1").arg(name);
        if (descriptionCache->type(name) == "xfile" && (plotFlags(name)& Plot_Backup))
        {
            foreach(QString df, plotSourceDataframes(name))
            {
                job->cmdList << QString("destroy %1").arg(df);
                // partial cleanup of df-plot data structures, needs proper solution
                m_plotsOfSourceDf.remove(df, name);
            }
            m_plotSourceDataframeSettings.remove(name);
        }
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job
                << recentlyDestroyedJob();
        submitJobs(jobs);
    }
}

void SessionManager::addDataframeMerge(QString df, QString dfm)
{
    if (!dataframeMergeOfSource.contains(df, dfm))
        dataframeMergeOfSource.insert(df, dfm);
}

void SessionManager::replacePlotSource(QString plot, QString settingsLabel, QString oldSourceDf, QString newSourceDf)
{
    m_plotsOfSourceDf.remove(oldSourceDf, plot);
    m_plotsOfSourceDf.insert(newSourceDf, plot);
    QMap<QString, QString> settings = m_plotSourceDataframeSettings.value(plot);
    settings.insert(settingsLabel, newSourceDf);
    m_plotSourceDataframeSettings.insert(plot, settings);
}

void SessionManager::setPlotFlags(QString name, int flags)
{
    m_plotFlags[name] = flags;
}

void SessionManager::observerEnabled(QString observer, bool enabled)
{
    if (observer.isEmpty())
    {
        observer = getDataFromJob(sender(), "observer").toString();
        if (observer.isEmpty())
        {
            qDebug() << "ERROR: SessionManager::observerEnabled observer tag is empty";
            return;
        }
        enabled  = getDataFromJob(sender(), "enabled").toBool();
    }
    if (enabled)
    {
        if (!m_enabledObservers.contains(observer))
                m_enabledObservers.append(observer);
    }
    else
        m_enabledObservers.removeAll(observer);
}

QStringList SessionManager::sourceDataframes(QString df)
{
    if (!exists(df))
        return QStringList();

    QSet<QString> sourceDfs({df});
    return sourceDfs.unite(matchListFromMap(dataframeMergeOfSource, df)).toList();
}

QStringList SessionManager::affectedPlots(QString resultsDf)
{
    QSet<QString> plots;
    QStringList dfList({resultsDf});
    if (!suspendingObservers() && !enabledObservers().isEmpty())
    {
        QStringList observerDfs = enabledObservers();
        observerDfs.replaceInStrings(QRegExp("^(.*)$"), "(\\1 default_dataframe)");
        dfList.append(observerDfs);
    }
    foreach (QString df, dfList)
        foreach(QString sourceDf, sourceDataframes(df))
            foreach (QString plot, m_plotsOfSourceDf.values(sourceDf))
                plots.insert(plot);
    return plots.toList();
}

bool SessionManager::isAnyTrialPlot(QString name)
{
    if (m_plotFlags.contains(name))
        return m_plotFlags.value(name) & Plot_AnyTrial;
    return false;
}

QMap<QString, QString> SessionManager::plotSourceDataframeSettings(QString plotName)
{
    return m_plotSourceDataframeSettings.value(plotName);
}

QString SessionManager::makeValidObjectName(QString name)
{
    return validator->makeValid(name);
}

bool SessionManager::isValidObjectName(QString name)
{
    return validator->isValid(name);
}

void SessionManager::addToExtraNamedItems(QString name)
{
    if (!extraNamedItems.contains(name))
        extraNamedItems.append(name);
}

void SessionManager::removeFromExtraNamedItems(QString name)
{
    extraNamedItems.removeAll(name);
}



void SessionManager::submitJobs(QList<LazyNutJob *> jobs)
{
    jobQueue->tryRun(jobs);
}

QVariant SessionManager::getDataFromJob(QObject *obj, QString key)
{
    LazyNutJob *job = qobject_cast<LazyNutJob *>(obj);
    if (!job)
    {
        qDebug() << "ERROR: SessionManager::getDataFromJob cannot extract LazyNutJob from sender";
        return QVariant();
    }
    QMap<QString, QVariant> data = job->data.toMap();
    if (!data.contains(key))
    {
        qDebug() << "ERROR: SessionManager::getDataFromJob LazyNutJob->data does not contain key entry" << key;
        return QVariant();
    }
    return data.value(key);
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

bool SessionManager::exists(QString name)
{
    if (!descriptionCache)
        return false;
    return descriptionCache->exists(name);
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
