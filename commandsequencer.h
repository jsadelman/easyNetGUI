#ifndef COMMANDSEQUENCER_H
#define COMMANDSEQUENCER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "jobqueue.h"
#include "enumclasses.h"

class LazyNut;

class CmdQueue: public JobQueue<QString,CmdQueue>
{
public:
    CmdQueue(LazyNut* lazyNut);
    void run(QString *cmd);
    void reset();
    QString name();

private:
    LazyNut* lazyNut;
};



class CommandSequencer: public QObject
{
    Q_OBJECT

public:
    CommandSequencer(LazyNut* lazyNut, QObject *parent=0);
    void currentReceivedCount();
    bool isPaused() {return cmdQueue->isPaused();}


public slots:
    void runCommands(QStringList commands,
                     JobOrigin origin,
                     SynchMode mode = SynchMode::Asynch
                    );
    void runCommand(QString command,
                    JobOrigin origin,
                    SynchMode mode = SynchMode::Asynch);
    void receiveResult(QString result);
    // status
    bool getStatus();
    int getCurrentReceivedCount();
    void pause();
    void stop();


signals:
    // send output to editor
    void commandExecuted(QString);
    void commandsExecuted();
    void currentReceivedCount(int);
    // states
    void isReady(bool);
    void cmdQueuePaused(bool);
    void cmdQueueStopped(bool);
    // errors
    void missingCmdError();

private:

    //int cmdCounter;
    JobOrigin jobOrigin;
    SynchMode synchMode;
    int sentCount;
    int receivedCount;
    bool ready;
    QString results;
    LazyNut* lazyNut;
    QStringList commandList;
    CmdQueue* cmdQueue;

    QRegExp cmdCountRex;
    QRegExp emptyLineRex;


};

#endif // COMMANDSEQUENCER_H
