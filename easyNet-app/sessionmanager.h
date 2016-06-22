#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>
#include <QObject>
#include <QSharedPointer>
#include "qprocess.h"
#include "enumclasses.h"

class QDomDocument;

class LazyNutJob;
class LazyNutJobParam;
template <class Job>
class JobQueue;
typedef JobQueue<LazyNutJob*> LazyNutJobQueue;


class QueryContext;
class TreeModel;
class TreeItem;
class AsLazyNutObject;
typedef QHash<QString,AsLazyNutObject*> LazyNutObjectCatalogue;
class ObjExplorer;
class DesignWindow;
class LazyNut;
class CommandSequencer;
class ObjectCache;
class ObjectCacheFilter;
class ObjectUpdater;
class ObjectNameValidator;


class SessionManager: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentModel READ currentModel WRITE setCurrentModel NOTIFY currentModelChanged)
    Q_PROPERTY(QString currentTrial READ currentTrial WRITE setCurrentTrial NOTIFY currentTrialChanged)
    Q_PROPERTY(QString currentSet READ currentSet WRITE setCurrentSet NOTIFY currentSetChanged)
    Q_PROPERTY(QString easyNetHome READ easyNetHome WRITE setEasyNetHome NOTIFY easyNetHomeChanged)
    Q_PROPERTY(QString easyNetDataHome READ easyNetDataHome WRITE setEasyNetDataHome NOTIFY easyNetDataHomeChanged)


friend class LazyNutJob;
friend class ObjectNameValidator;

public:
    static SessionManager* instance(); // singleton
    void startLazyNut();
    QString currentModel() {return m_currentModel;}
    QString currentTrial() {return m_currentTrial;}
    QString currentSet() {return m_currentSet;}
    QString easyNetHome() {return m_easyNetHome;}
    QString easyNetDataHome() {return m_easyNetDataHome;}
    QString easyNetUserHome() {return m_easyNetUserHome;}
    QString easyNetDir(QString env);
    QString defaultLocation(QString env) {return m_defaultLocation.value(env, QString());}
    QString nextPrettyName(QString type);


    void setEasyNetHome(QString dir);
    void setEasyNetDataHome(QString dir);
    void setEasyNetDir(QString env, QString dir);


    void submitJobs(QList<LazyNutJob*> jobs);
    void submitJobs(LazyNutJob* job) {submitJobs(QList<LazyNutJob*>{job});}
    QVariant getDataFromJob(QObject *obj, QString key);

    LazyNutJob* recentlyCreatedJob();
    LazyNutJob* recentlyModifiedJob();
    LazyNutJob* recentlyDestroyedJob();
    QList<LazyNutJob*> updateObjectCacheJobs();

    bool exists(QString name);
    QSet<QString> dependencies(QString name);
    QSet<QString> dataframeDependencies(QString name);

    QList<QSharedPointer<QDomDocument> > trialRunInfo(QString name);
    void setTrialRunInfo(QString df, QList<QSharedPointer<QDomDocument> > info);
    void setTrialRunInfo(QString df, QSharedPointer<QDomDocument> info);
    void appendTrialRunInfo(QString df, QList<QSharedPointer<QDomDocument> > info);
    void appendTrialRunInfo(QString df, QSharedPointer<QDomDocument> info);
    void clearTrialRunInfo(QString df);
    void removeTrialRunInfo(QString df);
    void copyTrialRunInfo(QString fromObj, QString toObj);


    QStringList enabledObservers() {return m_enabledObservers;}
    bool suspendingObservers() {return m_suspendingObservers;}
    bool isAnyTrialPlot(QString name);
    int plotFlags(QString name) {return m_plotFlags.value(name, 0);}
    QStringList lazyNutkeywords;
    QString makeValidObjectName(QString name);
    bool isValidObjectName(QString name);
    void addToExtraNamedItems(QString name);
    void removeFromExtraNamedItems(QString name);
    QStringList extraNamedItems();
    bool isCopyRequested(QString original);
    bool isModelStageCompleted() {return m_isModelStageUpdated;}
    QDomDocument *description(QString name);
    QString visibility(QString name);
    LazyNutJob *createDataViewJob(QString name, QString prettyName, QString subtype, QString Type,
                                          QMap<QString,QString> settings=QMap<QString,QString>());


    ObjectCache *descriptionCache;
    ObjectCache *dataframeCache;


    void setEasyNetUserHome(QString dir);
signals:

    // send output to editor
    void userLazyNutOutputReady(const QString&);

    void lazyNutStarted();
    void isReady(bool);
    void isPaused(bool);
    void cmdError(QString,QString);
    void cmdR(QString,QStringList);
    void commandExecuted(QString, QString);
    void jobExecuted();
    void commandSent(QString);
    void logCommand(QString);
    void commandsInJob(int);
    void dotsCount(int);
    void dotsExpect(int);
    void lazyNutMacroStarted();
    void lazyNutMacroFinished();

    void recentlyCreated(QDomDocument*);
    void recentlyModified(QDomDocument*);
    void recentlyDestroyed(QStringList);
    void commandsCompleted();

    void macroQueueStopped(bool);

    void lazyNutNotRunning();
    // convenient signals for the implementation of updateRecentlyModified()
    void updateObjectCatalogue(QDomDocument*);
    void updateDiagramScene();
    void currentModelChanged(QString);
     void currentTrialChanged(QString);
      void currentSetChanged(QString);

    void lazyNutFinished(bool);
    void easyNetUserHomeChanged();
    void easyNetDataHomeChanged();
    void easyNetHomeChanged();

    void resetExecuted();

public slots:

    bool isReady();
    bool isOn();
    void killLazyNut();
    void reset();
    void oobStop();
    void runCmd(QString cmd, unsigned int logMode = 0);
    void runCmd(QStringList cmd, unsigned int logMode = 0);
    void restartLazyNut();
    void setCurrentModel(QString s) {m_currentModel = s; emit currentModelChanged(m_currentModel);}
    void setCurrentTrial(QString s) {m_currentTrial = s; emit currentTrialChanged(m_currentTrial);}
    void setCurrentSet(QString s)   {m_currentSet = s;   emit currentSetChanged(m_currentSet);}
    void setPrettyName(QString name, QString prettyName, bool quiet = false);
    void destroyObject(QString name);
    void setPlotFlags(QString name, int flags);
    void observerEnabled(QString observer=QString(), bool enabled=false);
    void suspendObservers(bool suspending) {m_suspendingObservers = suspending;}
    void suspendObservers() {m_suspendingObservers = true;}
    void resumeObservers() {m_suspendingObservers = false;}
    void setCopyRequested(QString original);
    void clearCopyRequested(QString original = "");
    void createDataView(QString name, QString prettyName, QString subtype, QString Type,
                        QMap<QString,QString> settings=QMap<QString,QString>());
    void setShowHint(QString name, QString show);


private slots:

    void getOOB(const QString &lazyNutOutput);
    void startOOB(QString code="");
    void startCommandSequencer();
    void lazyNutProcessError(int error);
    void setDefaultLocations();
    void updateModelStageCompleted(QDomDocument* domDoc);

//    void macroStarted();
//    void macroEnded();

//    void sendLazyNutFinishedStatus(int, QProcess::ExitStatus);

private:

    SessionManager();
    SessionManager(SessionManager const&){}
    SessionManager& operator=(SessionManager const&){}
    static SessionManager* sessionManager;

    // locations
    QString m_easyNetHome;
    QString m_easyNetDataHome;
    QString m_easyNetUserHome;
    QMap<QString, QString> m_defaultLocation;

    QString         lazyNutExt;
    QString         binDir;
    QString         lazyNutBasename;
    QString         oobBaseName;

    // state
    QString m_currentModel;
    QString m_currentTrial;
    QString m_currentSet;

    LazyNutJobQueue *jobQueue;
    CommandSequencer *commandSequencer;
    LazyNut* lazyNut;
    QProcess *oob;
    QString lazyNutOutput;
    QStringList commandList;
    QString lazyNutHeaderBuffer;
    QRegExp OOBrex;
    QString OOBsecret;

    QMap <QString, QList<QSharedPointer<QDomDocument> > > trialRunInfoMap;
    QMap <QString, int> m_plotFlags;
    QStringList m_enabledObservers;
    bool        m_suspendingObservers;
    bool        killingLazyNut;
    bool        m_isModelStageUpdated;

    ObjectNameValidator *validator;
    QStringList m_extraNamedItems;
    QStringList m_requestedNames;
    QStringList m_requestedCopies;
    ObjectCacheFilter *objectListFilter;

    ObjectCacheFilter *modelFilter;
    ObjectUpdater *modelDescriptionUpdater;

    QMap<QString, int> itemCount;
};

#endif // SESSIONMANAGER_H
