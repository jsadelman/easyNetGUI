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

void CommandSequencer::runCommands(QStringList commands, SynchMode mode)
{
    commandList.clear();
    // clean up empty lines.
    // In Synch mode, empty lines won't trigger any output from lazyNut
    // hence they would stall cmdQueue.
    // In Asynch mode, empty lines won't increase receivedCount,
    // which would never reach commandList.size()
    // Note: carrying out this cleanup here breaks the advantage of QStringList implicit sharing.
    // Maybe better cleaning up when the list is created.

    foreach (QString command, commands)
    {
        if (!emptyLineRex.exactMatch(command))
            commandList.append(command);
    }

    if (commandList.size() == 0)
        return;

    qDebug() << "commandList size " << commandList.size();
    synchMode = mode;
    ready = false;
    emit isReady(ready);
    qDebug() << "BUSY";
    switch (mode)
    {
    case SynchMode::Synch:
        for (int i = 0; i < commandList.size(); ++i)
        {
            //commandList[i].prepend(QString("%1 ").arg(QString::number(++sentCount)));
            cmdQueue->tryRun(&commandList[i]);
        }
        break;
    case SynchMode::Asynch:
        foreach (QString command, commandList)
            lazyNut->sendCommand(command);
        break;
    default:
        break;
    }
}

void CommandSequencer::runCommand(QString command, SynchMode mode)
{
    runCommands(QStringList{command},mode);
}

void CommandSequencer::receiveResult(QString result)
{
    switch (synchMode)
    {
    case SynchMode::Synch:
        if (cmdQueue->jobsInQueue() == 0)
        {
            ready = true;
            emit isReady(ready);
            emit commandsExecuted();
        }
        cmdQueue->freeToRun();
        break;
    case SynchMode::Asynch:
        ++receivedCount;
        qDebug() << receivedCount;
        if (receivedCount == commandList.size())
        {
            receivedCount = 0;
            ready = true;
            emit isReady(ready);
            qDebug() << "READY";
            emit commandsExecuted();
        }
    default:
        break;
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
    if (synchMode == SynchMode::Synch)
    {
        cmdQueue->pause();
        emit cmdQueuePaused(cmdQueue->isPaused());
    }
}

void CommandSequencer::stop()
{
    if (synchMode == SynchMode::Synch)
    {
        cmdQueue->stop();
        emit cmdQueueStopped(cmdQueue->isStopped());
    }
    // send OOB
}

