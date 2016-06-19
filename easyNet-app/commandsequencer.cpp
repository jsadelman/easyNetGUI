#include "commandsequencer.h"
#include "lazynut.h"

#include <QDebug>
#include <sstream>

CommandSequencer::CommandSequencer(LazyNut *lazyNut, QObject *parent)
    : lazyNut(lazyNut), ready(true), logMode(0), QObject(parent),dotcount(0),beginLine(-1),timeMode(false),svgMode(false),
      bytesPending(0)
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
        // make sure a cmd is actually a single line, otherwise chop it in lines
        // otherwise parsing error will occur (actually a crash)
        // skip empty lines, which do not trigger any response from lazyNut
        // don't send comments, just are not necessary
        foreach (QString cmdLine, cmd.split(QRegExp("[\r\n]"),QString::SkipEmptyParts))
            if (!(cmdLine.startsWith("#") || emptyLineRex.exactMatch(cmdLine)))
                commandList.append(cmdLine);
    }
    if (commandList.size() == 0)
    {
        // the user has selected only empty lines (by mistake)
        // or an empty job was sent
//        qDebug() << "empty cmdList";
        emit jobExecuted();
        return;
    }
    ready = false;
    emit commandsInJob(commandList.size());
    emit isReady(ready);



    // send cmds to lazyNut without removing them from commandList
    // they will be removed when their resp. lazyNut output is received
    foreach (QString cmd, commandList)
    {
        emit commandSent(cmd);
        if (echoInterpreter(cmd) || logMode & ECHO_INTERPRETER)
            emit logCommand(cmd);

        lazyNut->sendCommand(cmd+"\n");
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
      if(lazyNutOutput[newoffset-1]=='\r') { cr_adj=1;}

      lazyNutIncompleteLine+=lazyNutOutput.midRef(offset,newoffset-offset-cr_adj);
      lazyNutLines.append(QString());
      lazyNutLines.back().swap(lazyNutIncompleteLine);
      processLazyNutLine();
      offset=newoffset+1;
  }
  lazyNutIncompleteLine+=lazyNutOutput.midRef(offset);

  // Dots typically occur on an incomplete line, at least in quiet mode.
  if(lazyNutIncompleteLine.startsWith("PROGRESS: ."))
  {
      int totDots=dotcount;
      int length = lazyNutIncompleteLine.length();
      for(int i=10; i < length && lazyNutIncompleteLine[i]=='.';++i)
          totDots++;
      emit dotsCount(totDots);
  }
}

void CommandSequencer::processLazyNutLine()
{
    const QString& line=lazyNutLines.back();

    // Consider special ANSWER cases first
    if(line.startsWith("PROGRESS: Expect "))
    {
        std::stringstream expectS(line.midRef(16).toString().toStdString());
        int expect;
        expectS>>expect;
        emit dotsExpect(expect);
    }
    else if(line.startsWith("PROGRESS: ."))
    {
        int length = line.length();
        for(int i=10; i < length && line[i]=='.';++i)
            dotcount++;
        emit dotsCount(dotcount);
    }
    else if(line.startsWith("ANSWER: SVG file of "))
    {
        int spoff=line.indexOf(' ',20);
        QStringRef bc=line.midRef(20,spoff-20);

        bytesPending=bc.toInt();
        svgMode=true;
    }
    else if(svgMode)
    {
        if(currentAnswer.length()<bytesPending)
            currentAnswer+=line+'\n';

        if(currentAnswer.length()>=bytesPending)
            svgMode=false;
    }
    else if(getAnswer && bytesPending==0 && line.startsWith("ANSWER: ")) // standard ANSWER case
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
                if(bytesPending>0&&currentAnswer.length()>bytesPending)
                {
                    currentAnswer.truncate(bytesPending);
                }
                emit answerReady(currentAnswer,currentCmd);
                currentAnswer.clear();
            }
            QString timeString=line.mid(pos+6);  // actually catch time
            lazyNutLines.clear();
            beginLine=-1;
            timeMode=false;
            bytesPending=0;
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
        emit cmdError(currentCmd,line.mid(7));
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

