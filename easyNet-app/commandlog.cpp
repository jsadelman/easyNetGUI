#include "commandlog.h"
#include "codeeditor.h"

#include <QToolBar>
#include <QTextCursor>
#include <QDebug>

CommandLog::CommandLog(QWidget *parent)
    : EditWindow(parent)
{
    fileToolBar->removeAction(newAct);
    fileToolBar->removeAction(openAct);
    editToolBar->removeAction(cutAct);
    editToolBar->removeAction(pasteAct);
}

CommandLog::~CommandLog()
{

}

void CommandLog::addText(QString txt)
{
    textEdit->moveCursor(QTextCursor::End);
    textEdit->appendPlainText(txt);
    textEdit->moveCursor(QTextCursor::End);
    currentLine = 1+textEdit->document()->blockCount();
}


QString CommandLog::getHistory(int shift)
{
    QTextCursor *cursor = new QTextCursor(textEdit->document());
    qDebug() << "Entered getHistory: line = " << currentLine << "pos = " << cursor->position();
    QString line = "";
    cursor->movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,currentLine-1);
    cursor->movePosition(QTextCursor::StartOfLine);
    qDebug() << "attempted move to start of current line: line = " << currentLine << "pos = " << cursor->position();
    if (shift<0)
    {
        qDebug() << "Subtracting 1 from currentLine";
        currentLine--;
        if (currentLine==0)
        {
            qDebug() << "looping and returning to last line: line = " << currentLine;
            currentLine = 1+textEdit->document()->blockCount();
            return("");
        }
        else if (currentLine==textEdit->document()->blockCount())
        {
            qDebug() << "on last line; end, then up: line = " << currentLine << "pos = " << cursor->position();
            cursor->movePosition(QTextCursor::EndOfLine);
            cursor->movePosition(QTextCursor::Up, QTextCursor::KeepAnchor, abs(shift));
        }
        else
        {
            qDebug() << "attempting move up one line: line = " << currentLine << "pos = " << cursor->position();
            cursor->movePosition(QTextCursor::Up, QTextCursor::KeepAnchor, abs(shift));
            cursor->movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, abs(shift));
        }
        line=cursor->selectedText();
        cursor->movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
        qDebug() << "attempted move to start of line: line = " << currentLine << "pos = " << cursor->position();
    }
    if (shift>0) // cursor down
    {
        if (currentLine>0)
            currentLine++;
        if (currentLine==(1+textEdit->document()->blockCount()))
        {
//            qDebug() << "on last line, just capturing the end: line = " << currentLine;
//            cursor->movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor, abs(shift));
            qDebug() << "on last line, return blank: line = " << currentLine;
            return("");

        }
        else if (currentLine>(textEdit->document()->blockCount()))
        {
            qDebug() << "looping round, grab first line: line = " << currentLine;
            currentLine=1;
            cursor->movePosition(QTextCursor::Start);
            cursor->movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, abs(shift));
        }
        else
        {
            if (currentLine>0)
                cursor->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);
            cursor->movePosition(QTextCursor::StartOfLine);
            cursor->movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        }
        qDebug() << "attempted move down one line: line = " << currentLine;
        line=cursor->selectedText();
    }
    return(line.trimmed());
}
