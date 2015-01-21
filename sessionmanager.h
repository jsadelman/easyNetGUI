#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>
#include <QObject>
#include <QStateMachine>

#include "jobqueue.h"
#include "enumclasses.h"

class Macro;
class QueryState;

class MacroQueue: public JobQueue<Macro,MacroQueue>
{
public:
    MacroQueue(){}
    void run(Macro *macro);
    void reset();
    QString name();
};

class QueryContext;
class TreeModel;
class TreeItem;
class LazyNutObj;
typedef QHash<QString,LazyNutObj*> LazyNutObjCatalogue;
class ObjExplorer;
namespace lazyNutOutput {
    class Driver;
}
class DesignWindow;
class LazyNut;
class CommandSequencer;

class SessionManager: public QObject
{
    Q_OBJECT
    //Q_PROPERTY(SynchMode synchMode READ getSynchMode WRITE setSynchMode NOTIFY synchModeChanged)
    //Q_ENUMS(SynchMode)

public:
    SessionManager(LazyNut* lazyNut, LazyNutObjCatalogue *objCatalogue, TreeModel *objTaxonomyModel, QObject *parent=0);
    const CommandSequencer * getCommandSequencer() const {return commandSequencer;}
    SynchMode getSynchMode() const {return synchMode;}
    void setSynchMode(SynchMode mode)
    {
        synchMode = mode;
        emit synchModeChanged(synchMode);
    }
//    void setJobOrigin(JobOrigin origin) {currentJobOrigin = origin;}
//    JobOrigin getJobOrigin() {return currentJobOrigin;}

signals:
    // decision signals
    void skipDescriptions();

    // send output to editor
    void commandExecuted(QString);

    void currentReceivedCount(int);
    void isReady(bool);
    void isPaused(bool);

    //void cmdQueuePaused(bool);
    void cmdQueueStopped(bool);
    //void macroQueuePaused(bool);
    void macroQueueStopped(bool);


    void synchModeChanged(SynchMode);

    void beginObjHashModified();
    void endObjHashModified();


public slots:

    // macros
    void runModel(QStringList cmdList);
    void runSelection(QStringList cmdList);

    // status
    bool getStatus();
    int getCurrentReceivedCount();

    // controls
    void pause();
    void stop();

    void setSynchMode(bool mode)
    {
        if (mode)
            synchMode = SynchMode::Synch;
        else
            synchMode = SynchMode::Asynch;
        emit synchModeChanged(synchMode);
    }


private slots:

    // general macro operations
    void macroStarted();
    void macroEnded();



    // lazyNut operations (entering a Macro state)
    void runCommands(JobOrigin jobOrigin);
    void getSubtypes(JobOrigin jobOrigin);
    void getRecentlyModified(JobOrigin jobOrigin);
    void clearRecentlyModified(JobOrigin jobOrigin);
    void getDescriptions(JobOrigin jobOrigin);

    // LazyNut output processing
    void parseLazyNutOutput(const QString & lazyNutOutput);
    void processLazyNutOutput(); // (exiting a Macro state)

private:

    Macro *buildMacro();
    QueryState *buildQueryState(Macro *macro);

    MacroQueue *macroQueue;
    CommandSequencer *commandSequencer;
    LazyNut* lazyNut;
    QueryContext* context;
    lazyNutOutput::Driver* driver;
    QStringList commandList;
    QStringList recentlyModified;
    QString lastResults;
    LazyNutObjCatalogue *objCatalogue;
    TreeModel* objTaxonomyModel;
    QString lazyNutBuffer;
    QRegExp rxEND;
    SynchMode synchMode;
    QStringList lazyNutObjTypes{"layer","connection","conversion","representation","pattern","steps","database","file","observer"};

};

// some helper classes that connect to SessionManager slots
// Probably a better choice would be to nest these classes inside SessionManager.
// However, currently MOC does not support nested classes.
// See: http://www.qtcentre.org/threads/59498-Nested-classes-and-problem-with-meta-object-system


class Macro: public QStateMachine
{
    Q_OBJECT

public:
    Macro(QObject *parent=0);
    void stop() {stopped = true;}
    bool isStopped() {return stopped;}


private:
    bool stopped;
};

class MacroState: public QState
{
    Q_OBJECT

public:
    MacroState(Macro *macro, JobOrigin jobOrigin = JobOrigin::User);


signals:
    void enteredWithJobOrigin(JobOrigin);

protected slots:
    void emitEnteredWithJobOrigin();

protected:
    Macro *macro;
    JobOrigin jobOrigin;
};

class UserState: public MacroState
{
    Q_OBJECT

public:
    UserState(Macro *macro, JobOrigin jobOrigin = JobOrigin::User);

private slots:
    void deleteIfStopped();
};

class GUIState: public MacroState
{
    Q_OBJECT

public:
    GUIState(Macro *macro, JobOrigin jobOrigin = JobOrigin::GUI);

};


class QueryState: public GUIState
{
    Q_OBJECT

public:
    QueryState(Macro *macro, JobOrigin jobOrigin = JobOrigin::GUI);

};

//class RunCommandsState: public QState
//{
//    Q_OBJECT

//public:
//    RunCommandsState(SessionManager *sm, QState *parent=0);

//private:
//    SessionManager *sessionManager;
//};

//class GetRecentlyModifiedState: public QState
//{
//    Q_OBJECT

//public:
//    GetRecentlyModifiedState(SessionManager *sm, QState *parent=0);

//private:
//    SessionManager *sessionManager;
//};

//class ClearRecentlyModifiedState: public QState
//{
//    Q_OBJECT

//public:
//    ClearRecentlyModifiedState(SessionManager *sm, QState *parent=0);

//private:
//    SessionManager *sessionManager;
//};

//class GetRecentlyModifiedAndClearState: public QState
//{
//    Q_OBJECT

//public:
//    GetRecentlyModifiedAndClearState(SessionManager *sm, QState *parent=0);

//private:
//    SessionManager *sessionManager;
//};

//class GetDescriptionsState: public QState
//{
//    Q_OBJECT

//public:
//    GetDescriptionsState(SessionManager *sm, QState *parent=0);

//private:
//    SessionManager *sessionManager;
//};

//class GetSubtypesState: public QState
//{
//    Q_OBJECT

//public:
//    GetSubtypesState(SessionManager *sm, QState *parent=0);

//private:
//    SessionManager *sessionManager;
//};

#endif // SESSIONMANAGER_H
