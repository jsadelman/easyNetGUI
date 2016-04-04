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
#include "xmlelement.h"



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


Q_DECLARE_METATYPE(QSharedPointer<QDomDocument> )


SessionManager* SessionManager::sessionManager = nullptr;

SessionManager *SessionManager::instance()
{
    return sessionManager ? sessionManager : (sessionManager = new SessionManager);
}

SessionManager::SessionManager()
    : lazyNutHeaderBuffer(""),
      lazyNutOutput(""),
      OOBrex("OOB secret: (\\w+)(?=\\r?\\n)"),
      m_plotFlags(),
      m_suspendingObservers(false),
      m_easyNetHome(""),
      m_easyNetDataHome(""),
      m_currentModel(""),
      m_currentTrial(""),
      m_currentSet(""),
      #if defined(__linux__)
      lazyNutExt("sh"),
      binDir("bin-linux"),
      #elif defined(__APPLE__)
      lazyNutExt("sh"),
      binDir("bin-mac"),
      #elif defined(_WIN32)
      lazyNutExt("bat"),
      binDir("bin")
      #endif

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

    connect(this, SIGNAL(easyNetHomeChanged()), this, SLOT(setDefaultLocations()));
    connect(this, SIGNAL(easyNetDataHomeChanged()), this, SLOT(setDefaultLocations()));

    m_defaultLocation.clear();
    lazyNutBasename = QString("lazyNut.%1").arg(lazyNutExt);

    lazyNutkeywords = QStringList()
                      << "query"
                      << "xml"
                      << "xmllint"
                      << "quietly"
                      << "loglevel"
                      << "CRASH"
                      << "recently_created"
                      << "clear_recently_created"
                      << "recently_destroyed"
                      << "clear_recently_destroyed"
                      << "recently_modified"
                      << "clear_recently_modified"
                      << "version"
                      << "shush"
                      << "unshush"
                      << "include"
                      << "create"
                      << "destroy"
                      << "load"
                      << "until"
                      << "if"
                      << "list"
                      << "facets"
                      << "loop"
                      << "less"
                      << "or"
                      << "and"
                      << "default_model"
                      << "set_default_model"
                      << "named_loop"
                      << "creators"
                      << "aesthetic"
                      << "stop"
                      << "watchlist"
                      << "limit_descriptions"
                      << "unlimit_descriptions"
                      << "set"
                      << "get"
                      << "unset"
                      << "R"
                         ;
    validator = new ObjectNameValidator(this, lazyNutkeywords);
}


void SessionManager::startLazyNut()
{
    QSettings settings("easyNet", "GUI");
    QString easyNetHome = QDir::toNativeSeparators(settings.value("easyNetHome","../..").toString());
    QString easyNetDataHome = QDir::toNativeSeparators(settings.value("easyNetDataHome",easyNetHome).toString());
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("EASYNET_HOME", easyNetHome);
    env.insert("EASYNET_DATA_HOME", easyNetDataHome);
    lazyNut->setProcessEnvironment(env);
    lazyNut->setWorkingDirectory(QFileInfo(defaultLocation("lazyNutBat")).absolutePath());
    lazyNut->setProgram(defaultLocation("lazyNutBat"));
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

QString SessionManager::easyNetDir(QString env)
{
    if (env == "easyNetHome")
        return easyNetHome();
    else if (env == "easyNetDataHome")
        return easyNetDataHome();
    return QString();
}

void SessionManager::setEasyNetHome(QString dir)
{
    m_easyNetHome = dir;
    QSettings settings("easyNet", "GUI");
    settings.setValue("easyNetHome", dir);
    emit easyNetHomeChanged();
}

void SessionManager::setEasyNetDataHome(QString dir)
{
    m_easyNetDataHome = dir;
    QSettings settings("easyNet", "GUI");
    settings.setValue("easyNetDataHome", dir);
    emit easyNetDataHomeChanged();
}

void SessionManager::setEasyNetDir(QString env, QString dir)
{
    if (env == "easyNetHome")
        setEasyNetHome(dir);
    else if (env == "easyNetDataHome")
        setEasyNetDataHome(dir);
    return;
}

void SessionManager::sendLazyNutCrash(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0 || exitStatus !=QProcess::NormalExit)
    {
//        qDebug() << "exit status " << exitCode << exitStatus;
     //   emit lazyNutCrash();
    }
}


void SessionManager::restartLazyNut()
{
    SessionManager::instance()->descriptionCache->clear();
    killLazyNut();
    lazyNut->waitForFinished();
    startLazyNut();
}

void SessionManager::setPrettyName(QString name, QString prettyName)
{
    if (!descriptionCache->exists(name))
        return;
    LazyNutJob *job = new LazyNutJob;
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
        job->cmdList << QString("destroy %1").arg(name);
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job
                << recentlyDestroyedJob();
        submitJobs(jobs);
    }
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
            eNerror << "observer tag is empty";
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

void SessionManager::setCopyRequested(QString original)
{
    if (!isCopyRequested(original))
        m_requestedCopies.append(original);
}

void SessionManager::clearCopyRequested(QString original)
{
    if (original.isEmpty())
    {
        QVariant v = SessionManager::instance()->getDataFromJob(sender(), "original");
        if (!v.canConvert<QString>())
        {
            eNerror << "cannot retrieve a valid string from original key in sender LazyNut job";
            return;
        }
        original = v.value<QString>();
    }
    if (original.isEmpty())
    {
        eNerror << "original is empty";
        return;
    }
    m_requestedCopies.removeAll(original);
}


bool SessionManager::isAnyTrialPlot(QString name)
{
    if (m_plotFlags.contains(name))
        return m_plotFlags.value(name) & Plot_AnyTrial;
    return false;
}


QString SessionManager::makeValidObjectName(QString name)
{
    QString validName = validator->makeValid(name);
    m_requestedNames.append(validName);
    return validName;
}

bool SessionManager::isValidObjectName(QString name)
{
    return validator->isValid(name);
}

void SessionManager::addToExtraNamedItems(QString name)
{
    if (!m_extraNamedItems.contains(name))
        m_extraNamedItems.append(name);
}

void SessionManager::removeFromExtraNamedItems(QString name)
{
    m_extraNamedItems.removeAll(name);
}

QStringList SessionManager::extraNamedItems()
{
    return m_extraNamedItems;
}

bool SessionManager::isCopyRequested(QString original)
{
    return m_requestedCopies.contains(original);
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
    job->cmdList = QStringList({"xml recently_created", "clear_recently_created"});
    job->setAnswerReceiver(this, SIGNAL(recentlyCreated(QDomDocument*)), AnswerFormatterType::XML);
    return job;
}

LazyNutJob *SessionManager::recentlyModifiedJob()
{
    LazyNutJob *job = new LazyNutJob();
    job->cmdList = QStringList({"xml recently_modified", "clear_recently_modified"});
    job->setAnswerReceiver(this, SIGNAL(recentlyModified(QStringList)), AnswerFormatterType::ListOfValues);
    return job;
}

LazyNutJob *SessionManager::recentlyDestroyedJob()
{
    LazyNutJob *job = new LazyNutJob();
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

QSet<QString> SessionManager::dependencies(QString name)
{
    QSet<QString> deps({name});
    QDomDocument *domDoc = descriptionCache->getDomDoc(name);
    if (domDoc)
    {
        foreach(QString dep, XMLelement(*domDoc)["Dependencies"].listValues())
        {
            deps.unite(dependencies(dep));
        }
    }
    return deps;
}

QSet<QString> SessionManager::dataframeDependencies(QString name)
{
    QSet<QString> deps;
    QDomDocument *domDoc = descriptionCache->getDomDoc(name);
    if (domDoc)
    {
        if (XMLelement(*domDoc)["type"]() == "dataframe" &&
            XMLelement(*domDoc)["subtype"]() != "dataframe_view")
        {
            deps << name;
        }
        foreach(QString dep, XMLelement(*domDoc)["Dependencies"].listValues())
        {
            deps.unite(dataframeDependencies(dep));
        }
    }
    return deps;
}

QList<QSharedPointer<QDomDocument> > SessionManager::trialRunInfo(QString name)
{
    if (trialRunInfoMap.contains(name))
        return trialRunInfoMap.value(name);

    QList<QSharedPointer<QDomDocument> > info;
    foreach (QString df, dataframeDependencies(name))
    {
        if (trialRunInfoMap.value(df).count() > 0)
            info.append(trialRunInfoMap.value(df));
    }
    return info;
}

void SessionManager::setTrialRunInfo(QString df, QList<QSharedPointer<QDomDocument> > info)
{
    trialRunInfoMap[df] = info;
}

void SessionManager::setTrialRunInfo(QString df, QSharedPointer<QDomDocument> info)
{
    setTrialRunInfo(df, QList<QSharedPointer<QDomDocument> >({info}));
}

void SessionManager::appendTrialRunInfo(QString df, QList<QSharedPointer<QDomDocument> > info)
{
    trialRunInfoMap[df].append(info);
}

void SessionManager::appendTrialRunInfo(QString df, QSharedPointer<QDomDocument> info)
{
    appendTrialRunInfo(df, QList<QSharedPointer<QDomDocument> >({info}));
}

void SessionManager::clearTrialRunInfo(QString df)
{
    if (trialRunInfoMap.contains(df))
        trialRunInfoMap[df].clear();
}

void SessionManager::removeTrialRunInfo(QString df)
{
    trialRunInfoMap.remove(df);
}

void SessionManager::copyTrialRunInfo(QString fromObj, QString toObj)
{
    trialRunInfoMap[toObj] = trialRunInfo(fromObj);
}

void SessionManager::getOOB(const QString &lazyNutOutput)
{
    lazyNutHeaderBuffer.append(lazyNutOutput);
    if (lazyNutHeaderBuffer.contains(OOBrex))
    {
        OOBsecret = OOBrex.cap(1);
        emit userLazyNutOutputReady(lazyNutHeaderBuffer.left(lazyNutHeaderBuffer.indexOf(OOBsecret) + OOBsecret.length()));
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
    connect(commandSequencer, SIGNAL(cmdR(QString,QStringList)),
            this, SIGNAL(cmdR(QString,QStringList)));
    connect(commandSequencer, SIGNAL(commandExecuted(QString,QString)),
            this, SIGNAL(commandExecuted(QString,QString)));
    connect(commandSequencer, SIGNAL(commandsInJob(int)),
            this, SIGNAL(commandsInJob(int)));
    connect(commandSequencer, SIGNAL(commandSent(QString)),
            this, SIGNAL(commandSent(QString)));
    connect(commandSequencer, SIGNAL(logCommand(QString)),
            this, SIGNAL(logCommand(QString)));
    connect(commandSequencer, SIGNAL(dotsCount(int)),
            this, SIGNAL(dotsCount(int)));



    emit isReady(commandSequencer->getStatus());
}

void SessionManager::lazyNutProcessError(int error)
{
    qDebug() << "lazyNut process cannot start. QProcess::ProcessError" << error;
}

void SessionManager::setDefaultLocations()
{
    m_defaultLocation["lazyNutBat"]   =   QString("%1/%2/nm_files/%3").arg(easyNetHome()).arg(binDir).arg(lazyNutBasename);
    m_defaultLocation["scriptsDir"]   =   QString("%1/Models").arg(easyNetDataHome());
    m_defaultLocation["testsDir"]     =   QString("%1/Tests").arg(easyNetDataHome());
    m_defaultLocation["trialsDir"]    =   QString("%1/Trials").arg(easyNetDataHome());
    m_defaultLocation["stimDir"]     =    QString("%1/Databases/Stimulus_files").arg(easyNetDataHome());
    m_defaultLocation["dfDir"]        =   QString("%1/Databases").arg(easyNetDataHome());
    m_defaultLocation["rPlotsDir"]    =   QString("%1/%2/R-library/plots").arg(easyNetHome()).arg(binDir);
    m_defaultLocation["outputDir"]    =   QString("%1/Output_files").arg(easyNetHome());
    m_defaultLocation["rDataframeViewsDir"]    =   QString("%1/%2/R-library/dataframe_views").arg(easyNetHome()).arg(binDir);
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


void SessionManager::runCmd(QString cmd, unsigned int logMode)
{
    runCmd(QStringList({cmd}), logMode);
}

void SessionManager::runCmd(QStringList cmd, unsigned int logMode)
{
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = cmd;
    job->logMode = logMode;
    submitJobs(job);

    bool echo = false;
    foreach (QString c, cmd)
    {
        echo |= commandSequencer->echoInterpreter(c);
    }
    if (echo)
        submitJobs(updateObjectCatalogueJobs());
}
