#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>
#include <QObject>
#include <QStateMachine>
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


class SessionManager: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentModel READ currentModel WRITE setCurrentModel NOTIFY currentModelChanged)
    Q_PROPERTY(QString currentTrial READ currentTrial WRITE setCurrentTrial)
    Q_PROPERTY(QString currentSet READ currentSet WRITE setCurrentSet)

friend class LazyNutJob;

public:
    static SessionManager* instance(); // singleton
    void startLazyNut(QString lazyNutBat);
    QString currentModel() {return m_currentModel;}
    QString currentTrial() {return m_currentTrial;}
    QString currentSet() {return m_currentSet;}

    void submitJobs(QList<LazyNutJob*> jobs);
    void submitJobs(LazyNutJob* job) {submitJobs(QList<LazyNutJob*>{job});}
    QVariant getDataFromJob(QObject *obj, QString key);

    LazyNutJob* recentlyCreatedJob();
    LazyNutJob* recentlyModifiedJob();
    LazyNutJob* recentlyDestroyedJob();
    QList<LazyNutJob*> updateObjectCatalogueJobs();

    bool exists(QString name);
    QStringList sourceDataframes(QString df);
    QStringList affectedPlots(QString resultsDf);
    QStringList enabledObservers() {return m_enabledObservers;}
    bool suspendingObservers() {return m_suspendingObservers;}
    bool isAnyTrialPlot(QString name);
    int plotFlags(QString name) {return m_plotFlags.value(name, 0);}
    QMap<QString, QString> plotSourceDataframeSettings(QString plotName);
    QStringList plotsOfSourceDf(QString df) {return m_plotsOfSourceDf.values(df);}
    QStringList plotSourceDataframes(QString plotName) {return plotSourceDataframeSettings(plotName).values();}


    ObjectCache *descriptionCache;
    ObjectCache *dataframeCache;


signals:

    // send output to editor
    void userLazyNutOutputReady(const QString&);

    void lazyNutStarted();
    void isReady(bool);
    void isPaused(bool);
    void cmdError(QString,QStringList);
    void commandExecuted(QString, QString);
    void commandSent(QString);
    void logCommand(QString);
    void commandsInJob(int);
    void lazyNutMacroStarted();
    void lazyNutMacroFinished();

    void recentlyCreated(QDomDocument*);
    void recentlyModified(QStringList);
    void recentlyDestroyed(QStringList);
    void commandsCompleted();

    void macroQueueStopped(bool);

    void lazyNutNotRunning();
    // convenient signals for the implementation of updateRecentlyModified()
    void updateObjectCatalogue(QDomDocument*);
    void updateDiagramScene();
    void currentModelChanged(QString);

    void lazyNutCrash();


public slots:

    // status
    bool isReady();
    bool isOn();

    // controls
//    void pause();
//    void stop();
    void killLazyNut();
//    void updateObjectCatalogue();
//    void updateRecentlyModified();
//    void getDescriptions();
//    void queryRecentlyCreated();
//    void queryRecentlyModified();
//    void queryRecentlyDestroyed();
    void runCmd(QString cmd);
    void runCmd(QStringList cmd);

    void restartLazyNut(QString lazyNutBat);

    void setCurrentModel(QString s) {m_currentModel = s; emit currentModelChanged(m_currentModel);}
    void setCurrentTrial(QString s) {m_currentTrial = s;}
    void setCurrentSet(QString s) {m_currentSet = s;}
    void setPrettyName(QString name, QString prettyName);
    void destroyObject(QString name);
    void addDataframeMerge(QString df, QString dfm);
    void replacePlotSource(QString plot, QString settingsLabel, QString oldSourceDf, QString newSourceDf);
    void setPlotFlags(QString name, int flags);
    void observerEnabled(QString observer=QString(), bool enabled=false);
    void suspendObservers(bool suspending) {m_suspendingObservers = suspending;}


private slots:

    void getOOB(const QString &lazyNutOutput);
    void startCommandSequencer();
    void lazyNutProcessError(int error);

//    void macroStarted();
//    void macroEnded();

    void sendLazyNutCrash(int, QProcess::ExitStatus);




private:

    SessionManager();
    SessionManager(SessionManager const&){}
    SessionManager& operator=(SessionManager const&){}
    static SessionManager* sessionManager;


    // state
    QString m_currentModel;
    QString m_currentTrial;
    QString m_currentSet;

    LazyNutJobQueue *jobQueue;
    CommandSequencer *commandSequencer;
    LazyNut* lazyNut;
    QString lazyNutOutput;
    QStringList commandList;
    QString lazyNutHeaderBuffer;
    QRegExp OOBrex;
    QString OOBsecret;

    QMap <QString, int> m_plotFlags;
    QMultiMap <QString, QString> m_plotsOfSourceDf; // <dataframe, rplots>
    QMultiMap <QString, QString> dataframeMergeOfSource; // <dataframe, dataframe_merges>
    QMap <QString, QMap<QString, QString> > m_plotSourceDataframeSettings; // <rplot <key, val> >
    QStringList m_enabledObservers;
    bool        m_suspendingObservers;


};

#endif // SESSIONMANAGER_H
