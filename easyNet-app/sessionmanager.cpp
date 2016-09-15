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
#include "objectupdater.h"
#include "easyNetMainWindow.h"
#include "objectnamevalidator.h"
#include "xmlelement.h"
#include <algorithm>

#include <iostream>
#include <cstdlib>

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
      OOBrex("OOB secret: (\\w+)\\r?\\n"),
      m_suspendingObservers(false),
      killingLazyNut(false),
      m_isModelStageCompleted(false),
      m_easyNetHome(""),
      m_easyNetDataHome(""),
      #if defined(__linux__)
      lazyNutExt("sh"),
      binDir("bin-linux"),
      oobBaseName(""),
      #elif defined(__APPLE__)
      lazyNutExt("sh"),
      binDir("bin-mac"),
      oobBaseName("lazyNut_oob"),
      #elif defined(_WIN32)
      lazyNutExt("bat"),
      binDir("bin"),
      oobBaseName("lazyNut_oob.exe"),
      #endif
      lazyNut(nullptr),
      commandSequencer(nullptr),
      oob(nullptr)
{
    jobQueue = new LazyNutJobQueue;

    descriptionCache = new ObjectCache(this);
    connect(this, SIGNAL(recentlyCreated(QDomDocument*)),
            descriptionCache, SLOT(create(QDomDocument*)));
    connect(this,  SIGNAL(recentlyModified(QDomDocument*)),
            descriptionCache,  SLOT(modify(QDomDocument*)));
    connect(this, SIGNAL(recentlyDestroyed(QStringList)),
            descriptionCache,  SLOT(destroy(QStringList)));

    dataframeCache = new ObjectCache(this);
    connect(this, SIGNAL(recentlyCreated(QDomDocument*)),
            dataframeCache, SLOT(create(QDomDocument*)));
    connect(this,  SIGNAL(recentlyModified(QDomDocument*)),
            dataframeCache,  SLOT(modify(QDomDocument*)));
    connect(this, SIGNAL(recentlyDestroyed(QStringList)),
            dataframeCache,  SLOT(destroy(QStringList)));

    modelFilter = new ObjectCacheFilter(descriptionCache, this);
    connect(this, SIGNAL(currentModelChanged(QString)),
            modelFilter, SLOT(setName(QString)));
    modelDescriptionUpdater = new ObjectUpdater(this);
    modelDescriptionUpdater->setProxyModel(modelFilter);
    connect(modelDescriptionUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)),
            this, SLOT(updateModelStageCompleted(QDomDocument*)));
    connect(this, SIGNAL(easyNetHomeChanged()), this, SLOT(setDefaultLocations()));
    connect(this, SIGNAL(easyNetDataHomeChanged()), this, SLOT(setDefaultLocations()));
    connect(this, SIGNAL(easyNetUserHomeChanged()), this, SLOT(setDefaultLocations()));

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
    m_currentModel = new ObjectCacheFilter(descriptionCache, this);
    m_currentTrial = new ObjectCacheFilter(descriptionCache, this);
    m_currentSet = new ObjectCacheFilter(descriptionCache, this);
    connect(m_currentModel, &ObjectCacheFilter::objectCreated, [=](QString name)
    {
        if (name == m_currentModel->first())
            emit currentModelChanged(name);
    });
    connect(m_currentModel, &ObjectCacheFilter::objectDestroyed, [=]()
    {
        emit currentModelChanged("");
    });
    connect(m_currentTrial, &ObjectCacheFilter::objectCreated, [=](QString name)
    {
        if (name == m_currentTrial->first())
            emit currentTrialChanged(name);
    });
    connect(m_currentTrial, &ObjectCacheFilter::objectDestroyed, [=]()
    {
        emit currentTrialChanged("");
    });
    connect(m_currentSet, &ObjectCacheFilter::objectCreated, [=](QString name)
    {
        if (name == m_currentSet->first())
            emit currentSetChanged(name);
    });
    connect(m_currentSet, &ObjectCacheFilter::objectDestroyed, [=]()
    {
        emit currentSetChanged("");
    });

    m_enabledObservers = new ObjectCacheFilter(descriptionCache, this);

}


void SessionManager::startLazyNut()
{
    disconnect(this,SIGNAL(resetExecuted()),this,SLOT(startLazyNut()));
    delete lazyNut;
    killingLazyNut = false;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QSettings settings("easyNet", "GUI");
    QString easyNetHome = QDir::toNativeSeparators(settings.value("easyNetHome","../..").toString());
    QString easyNetDataHome = QDir::toNativeSeparators(settings.value("easyNetDataHome",easyNetHome).toString());
    QString enuh=QString(
            #ifdef _WIN32
                getenv("USERPROFILE")
            #else
                getenv("HOME")
            #endif
                )+"/easyNet_files"
               ;
    QString easyNetUserHome = QDir::toNativeSeparators(settings.value("easyNetUserHome",
                                                                      enuh).toString());

    if(qEnvironmentVariableIsSet("EASYNET_HOME")) easyNetHome=qgetenv("EASYNET_HOME");
    if(qEnvironmentVariableIsSet("EASYNET_DATA_HOME")) easyNetDataHome=qgetenv("EASYNET_DATA_HOME");
    if(qEnvironmentVariableIsSet("EASYNET_USER_HOME")) easyNetUserHome=qgetenv("EASYNET_USER_HOME");
    env.insert("EASYNET_HOME", easyNetHome);
    env.insert("EASYNET_DATA_HOME", easyNetDataHome);
    env.insert("EASYNET_USER_HOME", easyNetUserHome);
    setEasyNetHome(easyNetHome);
    setEasyNetDataHome(easyNetDataHome);
    setEasyNetUserHome(easyNetUserHome);
    lazyNut = new LazyNut(this);
    connect(lazyNut, SIGNAL(started()), this, SLOT(startCommandSequencer()));
    connect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(getOOB(QString)));
    connect(lazyNut,  static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&LazyNut::finished), [=](int /*code*/, QProcess::ExitStatus status)
    {
        emit lazyNutFinished(!(killingLazyNut || (status == QProcess::NormalExit && lazyNut->exitCode()==0) ));
        killingLazyNut = false;
    });
    lazyNut->setProcessEnvironment(env);
    lazyNut->setWorkingDirectory(QFileInfo(defaultLocation("lazyNutBat")).absolutePath());
    lazyNut->setProgram(defaultLocation("lazyNutBat"));

    lazyNut->start();
    if (!lazyNut->waitForStarted())
    {
        qDebug() << lazyNut->program() <<
            #ifdef WIN32
                    lazyNut->nativeArguments()<<
            #endif
                    lazyNut->arguments() << lazyNut->error() << lazyNut->errorString();
        emit lazyNutNotRunning();
        eNerror << "lazyNut not running";
    }

}

QString SessionManager::currentModel()
{
    return m_currentModel->first();
}

QString SessionManager::currentTrial()
{
    return m_currentTrial->first();
}

QString SessionManager::currentSet()
{
    return m_currentSet->first();
}



QString SessionManager::easyNetDir(QString env)
{
    if (env == "easyNetHome")
        return easyNetHome();
    else if (env == "easyNetDataHome")
        return easyNetDataHome();
    else if (env == "easyNetUserHome")
        return easyNetUserHome();
    return QString();
}

QString SessionManager::nextPrettyName(QString type)
{
    if (!itemCount.contains(type))
        itemCount[type] = 1;
    return QString("%1 %2").arg(type).arg(itemCount[type]++);
}

QString SessionManager::addParenthesizedLetter(QString txt)
{
    if (prettyBaseNames.contains(txt))
        prettyBaseNames[txt] = prettyBaseNames[txt] < QChar('z').unicode() ?
                    prettyBaseNames[txt] + 1 : QChar('a').unicode();

    else
        prettyBaseNames[txt] = QChar('a').unicode();

    return QString("%1(%2)").arg(txt).arg(QChar(prettyBaseNames[txt]));
}

QString SessionManager::addParenthesizedNumber(QString txt)
{
    // used for pretty names only
    if (prettyBaseNames.contains(txt))
        prettyBaseNames[txt]++;
    else
        prettyBaseNames[txt] = 1;

    while (extraNamedItems().contains(QString("%1(%2)").arg(txt).arg(QString::number(prettyBaseNames[txt]))))
    {
        prettyBaseNames[txt]++;
    }
    return QString("%1(%2)").arg(txt).arg(QString::number(prettyBaseNames[txt]));
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

void SessionManager::setEasyNetUserHome(QString dir)
{
    m_easyNetUserHome = dir;
    QSettings settings("easyNet", "GUI");
    settings.setValue("easyNetUserHome", dir);
    emit easyNetUserHomeChanged();
}

QIcon SessionManager::viewIcon(int type, int state)
{
    return viewIconMap.value(type).value(state, QIcon());
}

unsigned int SessionManager::currentLogMode()
{
    return commandSequencer ? commandSequencer->logMode : 0;
}

void SessionManager::setEasyNetDir(QString env, QString dir)
{
    if (env == "easyNetHome")
        setEasyNetHome(dir);
    else if (env == "easyNetDataHome")
        setEasyNetDataHome(dir);
    else if (env == "easyNetUserHome")
        setEasyNetUserHome(dir);
    return;
}



void SessionManager::restartLazyNut()
{
    connect(this,SIGNAL(lazyNutKilled()),this,SLOT(reset()));
    connect(this,SIGNAL(resetExecuted()),this,SLOT(startLazyNut()));
    killLazyNut();
}

void SessionManager::setCurrentModel(QString name)
{
    m_currentModel->setName(name);
}

void SessionManager::setCurrentTrial(QString name)
{
    m_currentTrial->setName(name);
}

void SessionManager::setCurrentSet(QString name)
{
    m_currentSet->setName(name);
}

void SessionManager::setPrettyName(QString name, QString prettyName, bool quiet)
{
    if (!descriptionCache->exists(name))
        return;
    LazyNutJob *job = new LazyNutJob;
    QList<LazyNutJob *> jobs =  QList<LazyNutJob *> ()
                                << job;
    job->cmdList << QString("%1 set_pretty_name %2").arg(name).arg(prettyName);
    if (quiet)
        job->cmdList << "clear_recently_modified";
    else
        jobs << recentlyModifiedJob();

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
        if (!m_enabledObservers->contains(observer))
                m_enabledObservers->addName(observer);
    }
    else
        m_enabledObservers->removeName(observer);
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

void SessionManager::createDataView(QString name, QString prettyName, QString subtype, QString Type, QMap<QString, QString> settings)
{
    LazyNutJob *job = createDataViewJob(name, prettyName, subtype, Type, settings);
    if (!job)
        return;

    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->updateObjectCacheJobs();
    SessionManager::instance()->submitJobs(jobs);
}

void SessionManager::createTable(QString name, QString prettyName, QString Type, QMap<QString, QString> settings)
{
    createDataView(name, prettyName, "dataframe_view", Type, settings);
}

void SessionManager::createPlot(QString name, QString prettyName, QString Type, QMap<QString, QString> settings)
{
    createDataView(name, prettyName, "rplot", Type, settings);
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

QDomDocument *SessionManager::description(QString name)
{
    return descriptionCache->getDomDoc(name);
}

QString SessionManager::visibility(QString name)
{
    QString val;
    QDomDocument *desc = description(name);
    if (desc)
    {
        if (XMLelement(*desc)["hints"].listLabels().contains("show"))
            val = XMLelement(*desc)["hints"]["show"]();
    }
    return val;
}

LazyNutJob * SessionManager::createDataViewJob(QString name, QString prettyName, QString subtype, QString Type, QMap<QString, QString> settings)
{
    if (!(subtype == "dataframe_view" || subtype == "rplot"))
    {
        eNerror << "invalid subtype:" << subtype;
        return Q_NULLPTR;
    }
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({
                                   QString("create %1 %2").arg(subtype).arg(name),
                                   QString("%1 set_type %2").arg(name).arg(Type),
                                   QString("%1 set_pretty_name %2").arg(name).arg(prettyName)
                               });
   QMapIterator<QString, QString> settings_it(settings);
   while (settings_it.hasNext())
   {
       settings_it.next();
       job->cmdList << QString("%1 %2 %3 %4")
                       .arg(name)
                       .arg(SessionManager::instance()->exists(settings_it.value()) ? "setting_object" : "setting")
                       .arg(settings_it.key())
                       .arg(settings_it.value());
   }
   return job;
}



void SessionManager::submitJobs(QList<LazyNutJob *> jobs)
{
    if (lazyNut->state() == QProcess::Running)
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
    job->setAnswerReceiver(this, SIGNAL(recentlyModified(QDomDocument*)), AnswerFormatterType::XML);
    return job;
}

LazyNutJob *SessionManager::recentlyDestroyedJob()
{
    LazyNutJob *job = new LazyNutJob();
    job->cmdList = QStringList({"xml recently_destroyed", "clear_recently_destroyed"});
    job->setAnswerReceiver(this, SIGNAL(recentlyDestroyed(QStringList)), AnswerFormatterType::ListOfValues);
    return job;
}

QList<LazyNutJob *> SessionManager::updateObjectCacheJobs()
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

QStringList SessionManager::enabledObservers()
{
    return m_enabledObservers->names();
}

void SessionManager::getOOB(const QString &lazyNutOutput)
{
//    qDebug() << lazyNutOutput;

    lazyNutHeaderBuffer.append(lazyNutOutput);
    if (lazyNutHeaderBuffer.contains(OOBrex))
    {
        OOBsecret = OOBrex.cap(1);
        QString lazyNutIntro=lazyNutHeaderBuffer.left(lazyNutHeaderBuffer.indexOf(OOBrex)+OOBrex.matchedLength())+"\n";
//

        lazyNutHeaderBuffer=lazyNutHeaderBuffer.mid(lazyNutHeaderBuffer.indexOf(OOBsecret) + OOBsecret.length());
        disconnect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(getOOB(QString)));
        connect(lazyNut, SIGNAL(outputReady(QString)), commandSequencer, SLOT(processLazyNutOutput(QString)));
        emit userLazyNutOutputReady(lazyNutIntro);
        emit lazyNutStarted();
        lazyNut->outputReady(lazyNutHeaderBuffer);
        startOOB();
    }
}

void SessionManager::startOOB(QString code)
{
    if (code.isEmpty())
        code = OOBsecret;
    delete oob;
    oob = new QProcess(this);
    oob->setProgram(QString("%1/%2").arg(QFileInfo(defaultLocation("lazyNutBat")).absolutePath()).arg(oobBaseName));
    oob->setArguments({code});
    oob->start();
    if (!oob->waitForStarted())
        eNerror << "OOB did not start:";
}

void SessionManager::startCommandSequencer()
{
    delete commandSequencer;
    commandSequencer = new CommandSequencer(lazyNut, this);

    connect(commandSequencer,SIGNAL(userLazyNutOutputReady(QString)),
            this,SIGNAL(userLazyNutOutputReady(QString)));
    connect(commandSequencer, SIGNAL(isReady(bool)),
            this, SIGNAL(isReady(bool)));
    connect(commandSequencer, SIGNAL(commandsInJob(int)),
            this, SIGNAL(commandsInJob(int)));
    connect(commandSequencer, SIGNAL(jobExecuted()),
            this, SIGNAL(jobExecuted()));
    connect(commandSequencer, &CommandSequencer::cmdError, [=]()
    {
        jobQueue->clear();
//        submitJobs(updateObjectCacheJobs());
    });
    connect(commandSequencer, SIGNAL(cmdError(QString,QString)),
            this, SIGNAL(cmdError(QString,QString)));
    connect(commandSequencer, SIGNAL(cmdR(QString,QStringList)),
            this, SIGNAL(cmdR(QString,QStringList)));
    connect(commandSequencer, SIGNAL(commandExecuted(QString,QString)),
            this, SIGNAL(commandExecuted(QString,QString)));
    connect(commandSequencer, SIGNAL(cmdProcessingStarted(QString)),
            this, SIGNAL(cmdProcessingStarted(QString)));
    connect(commandSequencer, SIGNAL(commandSent(QString)),
            this, SIGNAL(commandSent(QString)));
    connect(commandSequencer, SIGNAL(logCommand(QString)),
            this, SIGNAL(logCommand(QString)));
    connect(commandSequencer, SIGNAL(dotsCount(int)),
            this, SIGNAL(dotsCount(int)));
    connect(commandSequencer, SIGNAL(dotsExpect(int)),
            this, SIGNAL(dotsExpect(int)));



    emit isReady(commandSequencer->getStatus());
}

void SessionManager::lazyNutProcessError(int error)
{
    qDebug() << "lazyNut process cannot start. QProcess::ProcessError" << error;
}

void SessionManager::setDefaultLocations()
{
    m_defaultLocation["lazyNutBat"]   =   QString("%1/%2/nm_files/%3").arg(easyNetHome()).arg(binDir).arg(lazyNutBasename);
    m_defaultLocation["modelsDir"]    =   QString("%1/Models").arg(easyNetUserHome());
    m_defaultLocation["scriptsDir"]   =   QString("%1/Scripts").arg(easyNetDataHome());
    m_defaultLocation["testsDir"]     =   QString("%1/Tests").arg(easyNetDataHome());
    m_defaultLocation["trialsDir"]    =   QString("%1/Trials").arg(easyNetDataHome());
    m_defaultLocation["stimDir"]      =   QString("%1/Databases/Stimulus_files").arg(easyNetDataHome());
    m_defaultLocation["dfDir"]        =   QString("%1/Databases").arg(easyNetDataHome());
    m_defaultLocation["rPlotsDir"]    =   QString("%1/bin/R-library/plots").arg(easyNetHome());
    m_defaultLocation["outputDir"]    =   QString("%1/Outputs").arg(easyNetUserHome());
    m_defaultLocation["docsDir"]    =   QString("%1/documentation").arg(easyNetDataHome());
    m_defaultLocation["docsImageDir"]    =   QString("%1/documentation/images").arg(easyNetHome());
    m_defaultLocation["rDataframeViewsDir"]    =   QString("%1/bin/R-library/dataframe_views").arg(easyNetHome());
    m_defaultLocation["docsDir"]   =   QString("%1/documentation").arg(easyNetDataHome());
}

void SessionManager::updateModelStageCompleted(QDomDocument *domDoc)
{
    QList<int> scriptList;
    foreach(QString s, XMLelement(*domDoc)["scripts"].listLabels())
        scriptList << s.toInt();
    bool completed = scriptList.isEmpty() ||
            XMLelement(*domDoc)["staging"]().toInt() > *std::max_element(scriptList.begin(), scriptList.end());

    if (!m_isModelStageCompleted && completed)
    {
        m_isModelStageCompleted = completed;
        emit modelStageCompleted();
    }
    else
        m_isModelStageCompleted = completed;
}


void SessionManager::setShowHint(QString name, QString show)
{
    if (!exists(name))
    {
//        eNerror << QString("object %1 does not exist").arg(name);
        return;
    }
    QDomDocument *desc = description(name);
    if (!desc)
    {
        eNwarning << QString("object %1 does not have a description, show hint will not be set.").arg(name);
        return;
    }
    QString cmd;

    if (!XMLelement(*desc)["hints"].listLabels().contains("show"))
        cmd = QString("%1 add_hint show %2").arg(name).arg(show);
    else //if (XMLelement(*desc)["hints"]["show"]() != show)
        cmd = QString("%1 change_hint show %2").arg(name).arg(show);

    if (!cmd.isEmpty())
    {
        LazyNutJob *job = new LazyNutJob;
        job->cmdList << cmd << "clear_recently_modified";
//        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
//                << job;
//        if (!quiet)
//                jobs << recentlyModifiedJob();
        submitJobs(job);
    }
}


void SessionManager::killLazyNut()
{
    killingLazyNut = true;

    if(oob) oob->write("stop\n");

    if(oob) oob->write("quit\n");
    QThread::sleep(1);
    if(oob->state()==QProcess::Running) oob->closeWriteChannel();
    if(lazyNut->state()==QProcess::Running)
    {
        lazyNut->write("stop\n");
        QTimer::singleShot(1000,Qt::CoarseTimer,this,SLOT(closeLazyNutChannels()));
    }else emit lazyNutKilled();
}

void SessionManager::closeLazyNutChannels()
{
  if(lazyNut->state()==QProcess::Running) { lazyNut->closeWriteChannel();
//lazyNut->closeReadChannel(QProcess::StandardOutput);
        QTimer::singleShot(1000,Qt::CoarseTimer,this,SLOT(reallyKillLazyNut()));
  }else emit lazyNutKilled();
}

void SessionManager::reallyKillLazyNut()
{
    if(lazyNut->state()==QProcess::Running) { qDebug()<<"going to have to kill lazyNut"; lazyNut->kill(); }

    emit lazyNutKilled();
}

void SessionManager::reset()
{
    disconnect(this,SIGNAL(lazyNutKilled()),this,SLOT(reset()));
    descriptionCache->clear();
    dataframeCache->clear();
    jobQueue->clear();
    jobQueue->forceFree();
    itemCount.clear();
    prettyBaseNames.clear();
    m_requestedNames.clear();
    m_isModelStageCompleted = false;
    emit resetExecuted();
}

void SessionManager::oobStop()
{
    if (oob)
        oob->write(qPrintable("stop\n"));
    jobQueue->clear();
    submitJobs(updateObjectCacheJobs());
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
        submitJobs(updateObjectCacheJobs());
}
