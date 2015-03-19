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
class LazyNutObject;
typedef QHash<QString,LazyNutObject*> LazyNutObjectCatalogue;
class ObjExplorer;
class DesignWindow;
class LazyNut;
class CommandSequencer;

class SessionManager: public QObject
{
    Q_OBJECT

public:
    SessionManager(QObject *parent=0);
    const CommandSequencer * getCommandSequencer() const {return commandSequencer;}
    void startLazyNut(QString lazyNutBat);
    void setupJob (QObject* sender, LazyNutJobParam* param);
    LazyNutJob *currentJob(QObject* sender);
    LazyNutJob* nextJob(QObject* sender);

signals:

    // send output to editor
    void userLazyNutOutputReady(const QString&);

    void isReady(bool);
    void isPaused(bool);

    void macroQueueStopped(bool);

    void lazyNutNotRunning();
    // convenient signals for the implementation of updateRecentlyModified()
    void updateLazyNutObjCatalogue(QDomDocument*);
    void updateDiagramScene();


public slots:

    // status
    bool getStatus();

    // controls
    void pause();
    void stop();
    void killLazyNut();

    void updateRecentlyModified();
    void getDescriptions();


private slots:

    void getOOB(const QString &lazyNutOutput);
    void startCommandSequencer();

    void macroStarted();
    void macroEnded();

    void appendCmdListOnNextJob(QStringList cmdList);



private:

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
