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
//    xmlCmdTags = QVector<QString>(LazyNutCommandTypes_MAX + 1);
//    xmlCmdTags[description] = "description";
//    xmlCmdTags[recently_modified] = "recently_modified";
//    xmlCmdTags[subtypes] = "subtypes";
    // etc..
    queryTypes << description << recently_modified << subtypes; // etc

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

void CommandSequencer::runCommand(QString command, JobOrigin origin)
{
    runCommands(QStringList{command}, origin);
}

void CommandSequencer::processLazyNutOutput(const QString &lazyNutOutput)
{
    if (jobOrigin == JobOrigin::User)
        emit userLazyNutOutputReady(lazyNutOutput); // display on console
    // else send it to some log file or other location
    lazyNutBuffer.append(lazyNutOutput);
    if (commandList.isEmpty())
        return; // startup header or other spontaneous lazyNut output, or synch error
    QString currentCmd = commandList.first();
    LazyNutCommandTypes currentCmdType;
    if (jobOrigin == JobOrigin::GUI)
    {
        if (currentCmd.contains("description"))
            currentCmdType = description;
        else if (currentCmd.contains("recently_modified"))
            currentCmdType = recently_modified;
        else if (currentCmd.contains("subtypes"))
            currentCmdType = subtypes;
        else
            ;// etc
    }

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
        if (jobOrigin == JobOrigin::GUI)
        {
            int answerOffset = answerRex.indexIn(lazyNutBuffer,beginOffset);
            if (beginOffset < answerOffset && answerOffset < endOffset)
            {
                QString answer = answerRex.cap(1);
                if (queryTypes.contains(currentCmdType))
                {
                    QDomDocument *domDoc = new QDomDocument;
                    domDoc->setContent(answer); // this line replaces an entire Bison!
                    if (currentCmdType == recently_modified)
                    {
                        // retreive obj name list and send it to SessionManager
                        // to be changed for new XML format
                        QStringList recentlyModified = domDoc->documentElement().text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
                        delete domDoc;
                        emit recentlyModifiedReady(recentlyModified);
                    }
                    else if (currentCmdType == subtypes)
                    {
                        // get the last word in the cmd line, e.g. xml subtypes layer
                        QString type = currentCmd.simplified().section(QRegExp("\\s+"),-1,-1);
                        // change <eNelements> into <string label="type" value="layer">
                        // stub
                        //qDebug() << type << domDoc->toString();
                        delete domDoc;
                    }
                    else if (currentCmdType == description)
                    {
                        emit descriptionReady(domDoc);
                    }
                    // else if ...
                }
                // else if R, else if SVG, process accordingly
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

