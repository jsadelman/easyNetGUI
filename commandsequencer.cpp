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
    qDebug() << "READY";
}

void CommandSequencer::runCommands(QStringList commands)
{
    ready = false;
    emit isReady(ready);
    //qDebug() << "BUSY";

    commandList.clear();
    commandList.append(commands);
    for (int i = 0; i < commandList.size(); ++i)
    {
        //commandList[i].prepend(QString("%1 ").arg(QString::number(++sentCount)));

        // skip empty lines, which won't trigger any output from lazyNut
        // hence they would stall cmdQueue
        if (!emptyLineRex.exactMatch(commandList[i]))
            cmdQueue->tryRun(&commandList[i]);
    }
}

void CommandSequencer::runCommand(QString command)
{
    runCommands(QStringList{command});
}

void CommandSequencer::receiveResult(QString result)
{
//    cmdCountRex.exactMatch(result);
//    int received = cmdCountRex.cap(1).toInt();
//    if (received == receivedCount + 1)
//        emit currentReceivedCount(++receivedCount);
//    else
//        qDebug() << "error, wrong cmd count";

//    emit commandExecuted(result);
//    results.append(result+"\n");

    //qDebug() << result;
    if (cmdQueue->jobsInQueue() == 0)
    {
//        emit resultsAvailable(results);
//        results.clear();
        ready = true;
        emit isReady(ready);
        //qDebug() << "READY";
        emit commandsExecuted();
    }
    cmdQueue->freeToRun();

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

