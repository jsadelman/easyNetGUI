#include "commandsequencer.h"
#include "lazynut.h"

#include <QDebug>


CommandSequencer::CommandSequencer(LazyNut *lazyNut, QObject *parent)
    : lazyNut(lazyNut), ready(true), logMode(0), QObject(parent)
{
    initProcessLazyNutOutput();
//    connect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(processLazyNutOutput(QString)));
}


void CommandSequencer::initProcessLazyNutOutput()
{
    beginRex = QRegExp("BEGIN: ([^\\r\\n]+)");
    emptyLineRex = QRegExp("^[\\s\\t]*$");
    errorRex = QRegExp("ERROR: ([^\\r\\n]*)"); //(?=\\n)
    rRex = QRegExp("\\bR: ([^\\r\\n]*)"); //(?=\\n)
//    answerRex = QRegExp("ANSWER: ([^\\r\\n]*)");//(?=\\n)
    answerRex = QRegExp("(ANSWER: [^\\n]*\\n)+");
    dotsRex = QRegExp("ANSWER: (\\.+)");

    eNelementsRex = QRegExp("<eNelements>");
    xmlStartRex = QRegExp("<(\\w+)");
    svgRex = QRegExp("SVG file of (\\d+) bytes:[ \\r\\n]*");
    answerDoneRex = QRegExp("ANSWER: Done");
    lazyNutBuffer.clear();
    baseOffset = 0;
}

bool CommandSequencer::echoInterpreter(QString cmd)
{
    return !(cmd.simplified().startsWith("xml") || cmd.contains("recently_") || cmd.contains(" get") || cmd.contains(" description"));
}


void CommandSequencer::runCommands(QStringList commands, bool _getAnswer, unsigned int mode)
{
    getAnswer = _getAnswer;
    logMode = mode;
//    QStringList commandsToRun;

    foreach (QString cmd, commands)
    {
        // skip empty lines, which do not trigger any response from lazyNut
        if ((!emptyLineRex.exactMatch(cmd)) && (!cmd.startsWith("#")))
            commandList.append(cmd);
    }
//    commandList.append(commandsToRun);

    if (commandList.size() == 0)
    {
        // the user has selected only empty lines (by mistake)
        // or an empty job was sent
        // (on purpose, e.g. for terminating a macro from a slot designated as next job slot)
//        qDebug() << "empty cmdList";
        emit jobExecuted();
        return;
    }
    ready = false;
    emit commandsInJob(commandList.size());
    emit isReady(ready);

//    qDebug() << "BUSY" << "first cmd: " << commandList.first();


    // send cmds to lazyNut without removing them from commandList
    // they will be removed when their resp. lazyNut output is received
    foreach (QString cmd, commandList)
    {
        emit commandSent(cmd);
        if (echoInterpreter(cmd) || logMode & ECHO_INTERPRETER)
            emit logCommand(cmd);
//        qDebug() << cmd;
        lazyNut->sendCommand(cmd);
    }
}

void CommandSequencer::runCommand(QString command, bool _getAnswer, unsigned int mode)
{
    runCommands(QStringList{command}, _getAnswer, mode);
}

void CommandSequencer::processLazyNutOutput(QString lazyNutOutput)
{
    lazyNutBuffer.append(lazyNutOutput);
    if (commandList.isEmpty())
    {
         emit userLazyNutOutputReady(lazyNutOutput);
//        lazyNutBuffer.clear();
        return; // startup header or empty job (no-op) or other spontaneous lazyNut output, or error
    }
    QString currentCmd, beginContent, timeString;
    int beginOffset, endOffset;
    while (true)
    {
        if (dotsRex.indexIn(lazyNutBuffer,baseOffset) > 0)
            emit dotsCount(dotsRex.cap(1).length());
        currentCmd = commandList.first();
        beginOffset = beginRex.indexIn(lazyNutBuffer,baseOffset);
        beginContent = beginRex.cap(1);
//        QRegExp endRex(QString("END: %1[^\\r\\n]*").arg(QRegExp::escape(lineNumber)));
        QRegExp endRex(QString("END: %1[^\\r\\n]*\\r?\\nINFO:[^\\r\\n]*took ([^\\r\\n]*)\\r?\\n").arg(QRegExp::escape(beginContent)));
        endOffset = endRex.indexIn(lazyNutBuffer,beginOffset);
        if (!(baseOffset <= beginOffset && beginOffset < endOffset))
        {
            if (echoInterpreter(currentCmd) || (logMode & ECHO_INTERPRETER))
                emit userLazyNutOutputReady(lazyNutOutput);
            return;
        }
        timeString = endRex.cap(1);
        int outputOffset = lazyNutBuffer.indexOf(lazyNutOutput);
        if (echoInterpreter(currentCmd) || (logMode & ECHO_INTERPRETER))
        {
            emit userLazyNutOutputReady(lazyNutOutput.left(endOffset + endRex.matchedLength() - outputOffset));
        }
        lazyNutOutput.remove(0, endOffset + endRex.matchedLength() - outputOffset);

        // extract ERROR lines
        int errorOffset = errorRex.indexIn(lazyNutBuffer,beginOffset);
        QStringList errorList = QStringList();
        while (beginOffset < errorOffset && errorOffset < endOffset)
        {
            errorList << errorRex.cap(0);
            errorOffset = errorRex.indexIn(lazyNutBuffer,errorOffset+1);
        }
        if (!errorList.isEmpty())
        {
//            clearCommandList();
            emit cmdError(currentCmd,errorList);
        }

        // extract R lines
        int rOffset = rRex.indexIn(lazyNutBuffer,beginOffset);
        QStringList rList = QStringList();
        while (beginOffset < rOffset && rOffset < endOffset)
        {
            rList << rRex.cap(0);
            rOffset = rRex.indexIn(lazyNutBuffer,rOffset+1);
        }
        if (!rList.isEmpty())
            emit cmdR(currentCmd,rList);

        if (getAnswer)
        {
            int answerOffset = answerRex.indexIn(lazyNutBuffer,beginOffset);
            if (beginOffset < answerOffset && answerOffset < endOffset)
            {
                QString answer = answerRex.cap(0);
                if (svgRex.indexIn(answer) > -1)
                {
                    int svgStart = answerOffset + answerRex.matchedLength();
                    int svgEnd = lazyNutBuffer.indexOf("ANSWER: Done.");
                    answer = lazyNutBuffer.mid(svgStart, svgEnd - svgStart);
                }
                answer = answer.remove("ANSWER: ");
//                qDebug() << currentCmd;
//                qDebug() << answer;
                emit answerReady(answer, currentCmd);
            }
        }
        emit commandExecuted(commandList.first(),timeString);
        commandList.removeFirst();
        baseOffset = endOffset + endRex.matchedLength();
        if (commandList.isEmpty())
        {
            baseOffset = 0;
            lazyNutBuffer.clear();
            ready = true;
            emit isReady(ready);
//            qDebug() << " emit commandsExecuted();";
            emit jobExecuted();
            return;
        }
    }
}




bool CommandSequencer::getStatus()
{
    return ready;
}

bool CommandSequencer::isOn()
{
    return lazyNut->state() == QProcess::Running;
}

void CommandSequencer::clearCommandList()
{
    if (commandList.length() > 1)
    {
        QStringList firstCmdOnlyList({commandList.first()});
        commandList.swap(firstCmdOnlyList);
    }
}

