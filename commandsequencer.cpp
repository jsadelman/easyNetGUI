#include "commandsequencer.h"
#include "lazynut.h"

#include <QDebug>


CommandSequencer::CommandSequencer(LazyNut *lazyNut, QObject *parent)
    : lazyNut(lazyNut), emptyLineRex(QRegExp("^[\\s\\t]*$")) ,ready(true),
      lazyNutBuffer(""), baseOffset(0), QObject(parent)
{
    connect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(receiveLazyNutOutput(QString)));
}

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
    // send cmds to lazyNut without removing them from commandList
    // they will be removed when their resp. lazyNut output is received
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


bool CommandSequencer::getStatus()
{
    return ready;
}

