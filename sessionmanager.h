#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>
#include <QObject>
#include <QStateMachine>

#include "jobqueue.h"
#include "enumclasses.h"


class MacroQueue: public JobQueue<QStateMachine,MacroQueue>
{
public:
    MacroQueue(){}
    void run(QStateMachine *macro);
    void reset();
    QString name();
};

class QueryContext;
class TreeModel;
class TreeItem;
class LazyNutObj;
typedef QHash<QString,LazyNutObj*> LazyNutObjCatalogue;
class ObjExplorer;
namespace lazyNutOutputParser {
    class Driver;
}
class DesignWindow;
class LazyNut;
class CommandSequencer;

class SessionManager: public QObject
{
    Q_OBJECT

public:
    SessionManager(LazyNutObjCatalogue *objCatalogue, TreeModel *objTaxonomyModel, QObject *parent=0);
    const CommandSequencer * getCommandSequencer() const {return commandSequencer;}

//    void setJobOrigin(JobOrigin origin) {currentJobOrigin = origin;}
//    JobOrigin getJobOrigin() {return currentJobOrigin;}
    void startLazyNut(QString lazyNutBat);

signals:
    // decision signals
    void skipDescriptions();

    // send output to editor
    void userLazyNutOutputReady(const QString&);
//    void commandExecuted(QString);
    // same signal as in CommandSequencer
    void commandsExecuted();
    void currentReceivedCount(int);
    void isReady(bool);
    void isPaused(bool);

    void macroQueueStopped(bool);



    void beginObjHashModified();
    void endObjHashModified();

    void lazyNutNotRunning();
    void lazyNutOutputParsed(bool);
    void lazyNutOutputProcessed();


public slots:

    // macros
    void runModel(QStringList cmdList);
    void runSelection(QStringList cmdList);

    // status
    bool getStatus();

    // controls
    void pause();
    void stop();
    void killLazyNut();



private slots:

    void getOOB(const QString &lazyNutOutput);
    void startCommandSequencer();
    void processLazyNutOutput(QString lno);

    // general macro operations
    void macroStarted();
    void macroEnded();



    // lazyNut operations (entering a Macro state)
    void runCommands();
    void getSubtypes();
    void getRecentlyModified();
    void clearRecentlyModified();
    void getDescriptions();

     void dispatchLazyNutOutput(QString lazyNutOutput, JobOrigin jobOrigin);

private:

    void initParser();
    void updateObjects();
    bool parseLazyNutOutput();

    QStateMachine *buildMacro();
    MacroQueue *macroQueue;
    CommandSequencer *commandSequencer;
    LazyNut* lazyNut;
    QString lazyNutOutput;

    QueryContext* context;
    lazyNutOutputParser::Driver* driver;
    QStringList commandList;
    QStringList recentlyModified;
    LazyNutObjCatalogue *objCatalogue;
    TreeModel* objTaxonomyModel;
    QString lazyNutHeaderBuffer;
    QRegExp OOBrex;
    QString OOBsecret;
    QStringList lazyNutObjTypes{"layer","connection","conversion","representation","pattern","steps","database","file","observer"};

};

#endif // SESSIONMANAGER_H
