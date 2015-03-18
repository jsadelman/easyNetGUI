#include "commandsequencer.h"
#include "lazynut.h"

#include <QDebug>


CommandSequencer::CommandSequencer(LazyNut *lazyNut, QObject *parent)
    : lazyNut(lazyNut), ready(true), logMode(ECHO_INTERPRETER), QObject(parent)
{
    initProcessLazyNutOutput();
    connect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(processLazyNutOutput(QString)));
}


void CommandSequencer::initProcessLazyNutOutput()
{
    emptyLineRex = QRegExp("^[\\s\\t]*$");
    errorRex = QRegExp("ERROR: ([^\\n]*)(?=\\n)");
    answerRex = QRegExp("ANSWER: ([^\\n]*)(?=\\n)");
    lazyNutBuffer.clear();
    baseOffset = 0;
}


void CommandSequencer::runCommands(QStringList commands, bool _getAnswer, unsigned int mode)
{
    getAnswer = _getAnswer;
    logMode = mode;

    foreach (QString cmd, commands)
    {
        // skip empty lines, which do not trigger any response from lazyNut
        if (!emptyLineRex.exactMatch(cmd))
            commandList.append(cmd);
    }

    if (commandList.size() == 0)
    {
        // likely the user has selected only empty lines (by mistake)
        emit commandsExecuted();
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

void CommandSequencer::runCommand(QString command, bool _getAnswer, unsigned int mode)
{
    runCommands(QStringList{command}, _getAnswer, mode);
}

void CommandSequencer::processLazyNutOutput(const QString &lazyNutOutput)
{
    if (logMode &= ECHO_INTERPRETER)
        emit userLazyNutOutputReady(lazyNutOutput);
    // else send it to some log file or other location
    lazyNutBuffer.append(lazyNutOutput);
    if (commandList.isEmpty())
        return; // startup header or other spontaneous lazyNut output, or synch error
    QString currentCmd = commandList.first();

    QRegExp beginRex(QString("BEGIN: %1\\n").arg(QRegExp::escape(currentCmd)));
    QRegExp endRex(QString("END: %1[^\\n]*\\n").arg(QRegExp::escape(currentCmd)));
    int beginOffset = beginRex.indexIn(lazyNutBuffer,baseOffset);
    int endOffset = endRex.indexIn(lazyNutBuffer,beginOffset);
    while (baseOffset <= beginOffset && beginOffset < endOffset)
    {
        // extract ERROR lines
        int errorOffset = errorRex.indexIn(lazyNutBuffer,beginOffset);
        QStringList errorList = QStringList();
        while (beginOffset < errorOffset && errorOffset < endOffset)
        {
            errorList << errorRex.cap(1);
            errorOffset = errorRex.indexIn(lazyNutBuffer,errorOffset+1);
        }
        if (!errorList.isEmpty())
            emit cmdError(currentCmd,errorList);

        // grab ANSWER (assume there's only one)
        if (getAnswer)
        {
            int answerOffset = answerRex.indexIn(lazyNutBuffer,beginOffset);
            if (beginOffset < answerOffset && answerOffset < endOffset)
            {
                QString answer = answerRex.cap(1);
                emit answerReady(answer);
            }
        }
        commandList.removeFirst();
        if (commandList.isEmpty())
        {
            emit commandsExecuted();
            baseOffset = 0;
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

