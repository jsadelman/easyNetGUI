#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>
#include <QObject>
#include <QStateMachine>


//#include "jobqueue.h"
#include "enumclasses.h"

class QDomDocument;
class LazyNutJob;
class LazyNutJobParam;
class MacroQueue;

//class MacroQueue: public JobQueue<QStateMachine,MacroQueue>
//{
//public:
//    MacroQueue(){}
//    void run(QStateMachine *macro);
//    void reset();
//    QString name();
//};

class QueryContext;
class TreeModel;
class TreeItem;
class AsLazyNutObject;
typedef QHash<QString,AsLazyNutObject*> LazyNutObjectCatalogue;
class ObjExplorer;
class DesignWindow;
class LazyNut;
class CommandSequencer;

class SessionManager: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentModel READ currentModel WRITE setCurrentModel)
    Q_PROPERTY(QString currentTrial READ currentTrial WRITE setCurrentTrial)
    Q_PROPERTY(QString currentPlot READ currentPlot WRITE setCurrentPlot)



public:
    static SessionManager* instance(); // singleton
    void startLazyNut(QString lazyNutBat);
    void setupJob (LazyNutJobParam* param, QObject* sender = nullptr);
    void setupNoOp(QObject* sender = nullptr);
    LazyNutJob *currentJob(QObject* sender);
    LazyNutJob* nextJob(QObject* sender);
    QString currentModel() {return m_currentModel;}
    QString currentTrial() {return m_currentTrial;}
    QString currentPlot() {return m_currentPlot;}


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


public slots:

    // status
    bool isReady();
    bool isOn();

    // controls
    void pause();
    void stop();
    void killLazyNut();
    void updateObjectCatalogue();
    void updateRecentlyModified();
    void getDescriptions();
    void queryRecentlyCreated();
    void queryRecentlyModified();
    void queryRecentlyDestroyed();
    void runCmd(QString cmd);
    void runCmd(QStringList cmd);

    void restartLazyNut(QString lazyNutBat);

    void setCurrentModel(QString s) {m_currentModel = s;}
    void setCurrentTrial(QString s) {m_currentTrial = s;}
    void setCurrentPlot(QString s) {m_currentPlot = s;}
private slots:

    void getOOB(const QString &lazyNutOutput);
    void startCommandSequencer();
    void lazyNutProcessError(int error);

    void macroStarted();
    void macroEnded();

    void appendCmdListOnNextJob(QStringList cmdList);




private:

    SessionManager();
    SessionManager(SessionManager const&){}
    SessionManager& operator=(SessionManager const&){}
    static SessionManager* sessionManager;

    // state
    QString m_currentModel;
    QString m_currentTrial;
    QString m_currentPlot; // not used

    MacroQueue *macroQueue;
    CommandSequencer *commandSequencer;
    LazyNut* lazyNut;
    QString lazyNutOutput;
    QStringList commandList;
    QString lazyNutHeaderBuffer;
    QRegExp OOBrex;
    QString OOBsecret;

};

#endif // SESSIONMANAGER_H
