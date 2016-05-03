#include "commandsequencer.h"
#include "lazynut.h"

#include <QDebug>


CommandSequencer::CommandSequencer(LazyNut *lazyNut, QObject *parent)
    : lazyNut(lazyNut), ready(true), logMode(0), QObject(parent),dotcount(0),beginLine(-1),timeMode(false),svgMode(false)
{
    initProcessLazyNutOutput();
//    connect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(processLazyNutOutput(QString)));
}


void CommandSequencer::initProcessLazyNutOutput()
{
    emptyLineRex = QRegExp("^[\\s\\t]*$");
    lazyNutLines.clear();
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

        lazyNut->sendCommand(cmd);
    }
}

void CommandSequencer::runCommand(QString command, bool _getAnswer, unsigned int mode)
{
    runCommands(QStringList{command}, _getAnswer, mode);
}

void CommandSequencer::processLazyNutOutput(QString lazyNutOutput)
{
  if (commandList.isEmpty() || echoInterpreter(commandList.first()) || (logMode & ECHO_INTERPRETER))
    emit userLazyNutOutputReady(lazyNutOutput);
  if(commandList.isEmpty()) return;

  int offset=0,newoffset=0;
  while( (newoffset=lazyNutOutput.indexOf('\n',offset)) >= 0 )
  {
      int cr_adj=0;
      if(lazyNutOutput[newoffset-1]=='\r') cr_adj=1;
      lazyNutIncompleteLine+=lazyNutOutput.midRef(offset,newoffset-offset-cr_adj);
      lazyNutLines.append(QString());
      lazyNutLines.back().swap(lazyNutIncompleteLine);
      processLazyNutLine();
      offset=newoffset+1;
  }
  lazyNutIncompleteLine+=lazyNutOutput.midRef(offset);

  // Dots typically occur on an incomplete line, at least in quiet mode.
  if(lazyNutIncompleteLine.startsWith("ANSWER: ."))
  {
      int totDots=dotcount;
      for(int i=8;lazyNutIncompleteLine[i]=='.';++i)
          totDots++;
      emit dotsCount(totDots);
  }
}

void CommandSequencer::processLazyNutLine()
{
    const QString& line=lazyNutLines.back();

    // Consider special ANSWER cases first
    if(line.startsWith("ANSWER: ."))
    {
        for(int i=8;line[i]=='.';++i)
            dotcount++;
        emit dotsCount(dotcount);
    }
    else if(line.startsWith("ANSWER: SVG file of "))
    {
        svgMode=true;
    }
    else if(svgMode)
    {
        if(line.startsWith("ANSWER: Done."))
            svgMode=false;
        else
            currentAnswer+=line;
    }
    else if(getAnswer && line.startsWith("ANSWER: ")) // standard ANSWER case
    {
        currentAnswer+=line.midRef(8);
    }
    else if(beginLine<0 && line.startsWith("BEGIN: "))
    {
        beginLine=lazyNutLines.size()-1;
        currentCmd=commandList.first();
    }
    else if(beginLine>=0 && line.startsWith("END:"))
    {
        dotcount=0;
        timeMode=true;
    }
    else if(timeMode && line.startsWith("INFO: "))
    {
        int pos;
        if((pos=line.lastIndexOf(" took"))>=0)
        {
            if(getAnswer)
            {
                emit answerReady(currentAnswer,currentCmd);
                currentAnswer.clear();
            }
            QString timeString=line.mid(pos+6);  // actually catch time
            lazyNutLines.clear();
            beginLine=-1;
            timeMode=false;
            emit commandExecuted(currentCmd,timeString);
            commandList.removeFirst();
            currentCmd.clear();
            if(commandList.isEmpty())
            {
              ready = true;
              emit isReady(ready);
              emit jobExecuted();
            }
        }
    }
    else if(line.startsWith("R: "))
    {
        emit cmdR(currentCmd,{currentCmd,line.mid(3),""});
    }
    else if(line.startsWith("ERROR: "))
    {
        emit cmdError(currentCmd,{currentCmd,line.mid(7),""});
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

