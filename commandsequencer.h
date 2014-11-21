#ifndef COMMANDSEQUENCER_H
#define COMMANDSEQUENCER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "jobqueue.h"

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

public slots:
    void runCommands(QStringList commands, bool synch = false);
    void runCommand(QString command);
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
    // mock results availability
    void resultsAvailable(QString);
    // ready state
    void isReady(bool);
    // errors
    void missingCmdError();

private:

    //int cmdCounter;
    bool synchMode;
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
