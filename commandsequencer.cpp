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
    beginRex = QRegExp("BEGIN: ([^\\r\\n]+)");
    emptyLineRex = QRegExp("^[\\s\\t]*$");
    errorRex = QRegExp("ERROR: ([^\\r\\n]*)"); //(?=\\n)
    answerRex = QRegExp("ANSWER: ([^\\r\\n]*)");//(?=\\n)
    eNelementsRex = QRegExp("<eNelements>");
    xmlStartRex = QRegExp("<(\\w+)");
    svgRex = QRegExp("SVG file of (\\d+) bytes:");
    answerDoneRex = QRegExp("ANSWER: Done\\.");//(?=\\n)
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
    emit commandsInJob(commandList.size());
    emit isReady(ready);

    qDebug() << "BUSY" << "first cmd: " << commandList.first();

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
    QString currentCmd, lineNumber;
    int beginOffset, endOffset;
    while (true)
    {
        currentCmd = commandList.first();
        beginOffset = beginRex.indexIn(lazyNutBuffer,baseOffset);
        lineNumber = beginRex.cap(1);
        QRegExp endRex(QString("END: %1[^\\r\\n]*").arg(QRegExp::escape(lineNumber)));
        endOffset = endRex.indexIn(lazyNutBuffer,beginOffset);
        if (!(baseOffset <= beginOffset && beginOffset < endOffset))
            return;

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
                if (svgRex.exactMatch(answer))
                {
                    int svgStart = answerOffset + answerRex.matchedLength();
                    int svgEnd = lazyNutBuffer.indexOf("</svg>", svgStart) + QString("</svg>").length();
                    answer = lazyNutBuffer.mid(svgStart, svgEnd - svgStart).remove(QRegExp("^[\\r\\n]*"));
                }
                else if (xmlStartRex.indexIn(answer) > -1)
                {
                    QRegExp xmlEndRex(QString("</%1\\s*>").arg(QRegExp::escape(xmlStartRex.cap(1))));
                    int xmlEnd = xmlEndRex.indexIn(lazyNutBuffer, answerOffset) + xmlEndRex.matchedLength();
                    answer = lazyNutBuffer.mid(answerOffset, xmlEnd - answerOffset).remove("ANSWER:");
                }
                emit answerReady(answer, currentCmd);
            }
        }
        emit commandExecuted(commandList.first());
        commandList.removeFirst();
        baseOffset = endOffset + endRex.matchedLength();
        if (commandList.isEmpty())
        {
            baseOffset = 0;
            lazyNutBuffer.clear();
            ready = true;
            emit isReady(ready);
            qDebug() << "READY";
            emit commandsExecuted();
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

