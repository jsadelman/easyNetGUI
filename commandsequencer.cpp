#include "commandsequencer.h"
#include "lazynut.h"

#include <QDebug>

CmdQueue::CmdQueue(LazyNut *lazyNut)
    : lazyNut(lazyNut)
{
}

void CmdQueue::run(QString *cmd)
{
    lazyNut->sendCommand(*cmd);
}

void CmdQueue::reset()
{
    //qDebug() << "CmdQueue::reset()";
    queue.clear();
}

QString CmdQueue::name()
{
    return "CmdQueue";
}


CommandSequencer::CommandSequencer(LazyNut *lazyNut, QObject *parent)
    : lazyNut(lazyNut), QObject(parent)
{
    receivedCount = 0;
    sentCount = 0;
    cmdCountRex = QRegExp("^(\\d+) ");
    emptyLineRex = QRegExp("^[\\s\\t]*$");
    cmdQueue = new CmdQueue(lazyNut);
    ready = true;
    //qDebug() << "READY";
}

void CommandSequencer::runCommands(QStringList commands, bool synch)
{
    commandList.clear();
    // clean up empty lines.
    // In sinch mode, empty lines won't trigger any output from lazyNut
    // hence they would stall cmdQueue.
    // In non-synch mode, empty lines won't increase receivedCount,
    // which would never reach commandList.size()
    foreach (QString command, commands)
    {
        if (!emptyLineRex.exactMatch(command))
            commandList.append(command);
    }

    if (commandList.size() == 0)
        return;

    synchMode = synch;
    ready = false;
    emit isReady(ready);
    //qDebug() << "BUSY";

    if (synchMode)
    {
        for (int i = 0; i < commandList.size(); ++i)
        {
            //commandList[i].prepend(QString("%1 ").arg(QString::number(++sentCount)));
            cmdQueue->tryRun(&commandList[i]);
        }
    }
    else
    {
        foreach (QString command, commandList)
            lazyNut->sendCommand(command);
    }
}

void CommandSequencer::runCommand(QString command)
{
    runCommands(QStringList{command});
}

void CommandSequencer::receiveResult(QString result)
{
    if (synchMode)
    {
        if (cmdQueue->jobsInQueue() == 0)
        {
            ready = true;
            emit isReady(ready);
            emit commandsExecuted();
        }
        cmdQueue->freeToRun();
    }
    else
    {
        ++receivedCount;
        qDebug() << receivedCount;
        if (receivedCount == commandList.size())
        {
            receivedCount = 0;
            ready = true;
            emit isReady(ready);
            //qDebug() << "READY";
            emit commandsExecuted();
        }
    }
}

bool CommandSequencer::getStatus()
{
    return ready;
}

int CommandSequencer::getCurrentReceivedCount()
{
    return receivedCount;
}

void CommandSequencer::pause()
{
    cmdQueue->pause();
}

void CommandSequencer::stop()
{
    cmdQueue->stop();
}

