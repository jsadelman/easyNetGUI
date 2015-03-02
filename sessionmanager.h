#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>
#include <QObject>
#include <QStateMachine>


#include "jobqueue.h"
#include "enumclasses.h"

class QDomDocument;


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

signals:
    // decision signals
    void skipDescriptions();

    // send output to editor
    void userLazyNutOutputReady(const QString&);

    void isReady(bool);
    void isPaused(bool);

    void macroQueueStopped(bool);

    void descriptionReady(QDomDocument*);
    void updateDiagramScene();


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

    // general macro operations
    void macroStarted();
    void macroEnded();


    // lazyNut operations (entering a Macro state)
    void runCommands();
    void getSubtypes();
    void getRecentlyModified();
    void clearRecentlyModified();
    void getDescriptions();


     void updateRecentlyModified(QDomDocument*dom);

private:

    QStringList extrctRecentlyModifiedList(QDomDocument* domDoc);

    void initParser();
    void updateObjects();
    bool parseLazyNutOutput();

    QStateMachine *buildMacro();
    MacroQueue *macroQueue;
public:
    CommandSequencer *commandSequencer;
private:
    LazyNut* lazyNut;
    QString lazyNutOutput;

    QStringList commandList;
    QStringList recentlyModified;
    LazyNutObjectCatalogue *objectCatalogue;
    TreeModel* objTaxonomyModel;
    QString lazyNutHeaderBuffer;
    QRegExp OOBrex;
    QString OOBsecret;
    QStringList lazyNutObjTypes{"layer","connection","conversion","representation","pattern","steps","database","file","observer"};

};

#endif // SESSIONMANAGER_H
