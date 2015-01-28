#include "commandsequencer.h"
#include "lazynut.h"

#include <QDebug>

//CmdQueue::CmdQueue(LazyNut *lazyNut)
//    : lazyNut(lazyNut)
//{
//}

//void CmdQueue::run(QString *cmd)
//{
//    lazyNut->sendCommand(*cmd);
//}

//void CmdQueue::reset()
//{
//    queue.clear();
//}

//QString CmdQueue::name()
//{
//    return "CmdQueue";
//}


CommandSequencer::CommandSequencer(LazyNut *lazyNut, QObject *parent)
    : lazyNut(lazyNut), emptyLineRex(QRegExp("^[\\s\\t]*$")) ,ready(true),
      lazyNutBuffer(""), baseOffset(0), QObject(parent)
{
    connect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(receiveLazyNutOutput(QString)));
//    receivedCount = 0;
//    sentCount = 0;
    //cmdCountRex = QRegExp("^(\\d+) ");
//    emptyLineRex = QRegExp("^[\\s\\t]*$");
//    cmdQueue = new CmdQueue(lazyNut);
//    ready = true;
    //qDebug() << "READY";
}

//void CommandSequencer::runCommands(QStringList commands, JobOrigin origin, SynchMode mode)
//{
//    commandList.clear();
//    // clean up empty lines.
//    // In Synch mode, empty lines won't trigger any output from lazyNut
//    // hence they would stall cmdQueue.
//    // In Asynch mode, empty lines won't increase receivedCount,
//    // which would never reach commandList.size()
//    // Note: carrying out this cleanup here breaks the advantage of QStringList implicit sharing.
//    // Maybe better cleaning up when the list is created.

//    foreach (QString command, commands)
//    {
//        if (!emptyLineRex.exactMatch(command))
//            commandList.append(command);
//    }

//    if (commandList.size() == 0)
//    {
//        emit commandsExecuted();
//        return;
//    }

//    //qDebug() << "commandList size " << commandList.size();
//    jobOrigin = origin;
//    synchMode = mode;
//    ready = false;
//    emit isReady(ready);
//    qDebug() << "BUSY";
//    switch (mode)
//    {
//    case SynchMode::Synch:
//        for (int i = 0; i < commandList.size(); ++i)
//        {
//            //commandList[i].prepend(QString("%1 ").arg(QString::number(++sentCount)));
//            cmdQueue->tryRun(&commandList[i]);
//        }
//        break;
//    case SynchMode::Asynch:
//        foreach (QString command, commandList)
//            lazyNut->sendCommand(command);
//        break;
//    default:
//        break;
//    }
//}

//void CommandSequencer::runCommand(QString command, JobOrigin origin, SynchMode mode)
//{
//    runCommands(QStringList{command}, origin, mode);
//}

void CommandSequencer::runCommands(QStringList commands, JobOrigin origin)
{
    jobOrigin = origin;
    if (jobOrigin == JobOrigin::User)
    {
        foreach (QString cmd, commands)
        {
            // skip empty lines, which do not trigger any response from lazyNut
            if (!emptyLineRex.exactMatch(cmd))
                commandList.append(cmd);
        }
    }
    else
        commandList = commands;
    if (commandList.size() == 0)
    {
        // likely the user has selected only empty lines (by mistake)
        emit commandsExecuted("",jobOrigin);
        return;
    }
    ready = false;
    emit isReady(ready);
    qDebug() << "BUSY";
    // send cmds to lazyNut without dequeueing them
    // they will be dequeued when their resp. lazyNut output is received
    foreach (QString cmd, commandList)
        lazyNut->sendCommand(cmd);
}

void CommandSequencer::runCommand(QString command, JobOrigin origin)
{
    runCommands(QStringList{command}, origin);
}

void CommandSequencer::receiveLazyNutOutput(const QString &lazyNutOutput)
{
    if (jobOrigin == JobOrigin::User)
        emit userLazyNutOutputReady(lazyNutOutput);
    // else send it to some log file or other location
    lazyNutBuffer.append(lazyNutOutput);
    if (commandList.isEmpty())
        return; // maybe error, maybe used for startup header
    QString currentCmd = commandList.first();
    QRegExp beginRex(QString("BEGIN: %1\\n").arg(QRegExp::escape(currentCmd)));
    QRegExp endRex(QString("END: %1[^\\n]*\\n").arg(QRegExp::escape(currentCmd)));
    QRegExp errorRex("ERROR: [^\\n]*(?=\\n)");
    int beginOffset = beginRex.indexIn(lazyNutBuffer,baseOffset);
    int endOffset = endRex.indexIn(lazyNutBuffer,beginOffset);
    while (baseOffset <= beginOffset && beginOffset < endOffset)
    {
        // signal errors
        int errorOffset = errorRex.indexIn(lazyNutBuffer,beginOffset);
        QStringList errorList = QStringList();
        while (beginOffset < errorOffset && errorOffset < endOffset)
        {
            errorList << errorRex.cap();
            errorOffset = errorRex.indexIn(lazyNutBuffer,errorOffset+1);
        }
        if (!errorList.isEmpty())
            emit cmdError(currentCmd,errorList);

        // tick current cmd
        qDebug() << commandList.size() << commandList.first();

        commandList.removeFirst();
        if (commandList.isEmpty())
        {
            baseOffset = 0;
            emit commandsExecuted(lazyNutBuffer,jobOrigin);
            lazyNutBuffer.clear();
            ready = true;
            emit isReady(ready);
            qDebug() << "READY";
            return;
        }
        currentCmd = commandList.first();
        baseOffset = endOffset + endRex.matchedLength();
        beginRex = QRegExp(QString("BEGIN: %1\\n").arg(QRegExp::escape(currentCmd)));
        endRex = QRegExp(QString("END: %1[^\\n]*\\n").arg(QRegExp::escape(currentCmd)));
        beginOffset = beginRex.indexIn(lazyNutBuffer,baseOffset);
        endOffset = endRex.indexIn(lazyNutBuffer,beginOffset);
    }
}





//void CommandSequencer::receiveResult(QString result)
//{
//    switch (synchMode)
//    {
//    case SynchMode::Synch:
//        if (cmdQueue->jobsInQueue() == 0)
//        {
//            ready = true;
//            emit isReady(ready);
//            emit commandsExecuted();
//        }
//        cmdQueue->freeToRun();
//        break;
//    case SynchMode::Asynch:
//        ++receivedCount;
//        //qDebug() << receivedCount;
//        if (receivedCount == commandList.size() || commandList.size() == 0)
//        {
//            receivedCount = 0;
//            ready = true;
//            emit isReady(ready);
//            qDebug() << "READY";
//            emit commandsExecuted();
//        }
//    default:
//        break;
//    }
//}

bool CommandSequencer::getStatus()
{
    return ready;
}

//int CommandSequencer::getCurrentReceivedCount()
//{
//    return receivedCount;
//}

//void CommandSequencer::pause()
//{
//    if (jobOrigin == JobOrigin::User && synchMode == SynchMode::Synch)
//        cmdQueue->pause();
//}

//void CommandSequencer::stop()
//{
//    if (jobOrigin == JobOrigin::User)
//    {
//        if (synchMode == SynchMode::Synch)
//            cmdQueue->stop();
// //        else if (synchMode == SynchMode::Asynch)
// //            commandList.clear();

//        qDebug () << "OOB sent";
//    }
//}

