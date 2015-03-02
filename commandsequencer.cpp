#include "commandsequencer.h"
#include "lazynut.h"

#include <QDebug>
#include <QDomDocument>


CommandSequencer::CommandSequencer(LazyNut *lazyNut, QObject *parent)
    : lazyNut(lazyNut), ready(true), QObject(parent)
{
    initProcessLazyNutOutput();
    connect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(processLazyNutOutput(QString)));
}


void CommandSequencer::initProcessLazyNutOutput()
{
    jobOrigin = JobOrigin::User;
    emptyLineRex = QRegExp("^[\\s\\t]*$");
    errorRex = QRegExp("ERROR: ([^\\n]*)(?=\\n)");
    answerRex = QRegExp("ANSWER: ([^\\n]*)(?=\\n)");
//    eNelementsTagRex = QRegExp("(</?)eNelements(>)");
    lazyNutBuffer.clear();
    baseOffset = 0;

}

void CommandSequencer::runCommands(QStringList commands, JobOrigin origin, QObject *obj, bool xml,char const* slot)
{
    jobOrigin = origin;
    QStringList clean_commands;
    for (int i=0; i<commands.size(); ++i)
        if (!emptyLineRex.exactMatch(commands[i])) clean_commands.append(commands[i]);
    for (int i=0; i<clean_commands.size(); ++i)
    {
            if(i!=clean_commands.size()-1)
            {
                commandList.append(new LazyNutCommand(clean_commands[i]));
            }
            else
            {
                commandList.append(new LazyNutCommand(clean_commands[i],obj,slot,xml));
            }
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
    foreach (LazyNutCommand* cmd, commandList)
        lazyNut->sendCommand(*cmd);
}

void CommandSequencer::runCommand(QString command, JobOrigin origin, QObject*obj, bool xml, char const* slot)
{
    runCommands(QStringList{command}, origin, obj, xml, slot);
}

void CommandSequencer::processLazyNutOutput(const QString &lazyNutOutput)
{
    if (jobOrigin == JobOrigin::User)
        emit userLazyNutOutputReady(lazyNutOutput); // display on console
    // else send it to some log file or other location
    lazyNutBuffer.append(lazyNutOutput);
    if (commandList.isEmpty())
        return; // startup header or other spontaneous lazyNut output, or synch error
    QString currentCmd = *commandList.first();

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
        QString answer;
        // grab ANSWER (assume there's only one)
        if (jobOrigin == JobOrigin::GUI)
        {
            int answerOffset = answerRex.indexIn(lazyNutBuffer,beginOffset);
            if (beginOffset < answerOffset && answerOffset < endOffset)
            {
                answer = answerRex.cap(1);
            }
        }
        commandList.first()->done(answer);
        delete commandList.first();
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
        currentCmd = *commandList.first();
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

