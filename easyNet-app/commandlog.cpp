#include "commandlog.h"
#include "codeeditor.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "objectcache.h"


#include "sessionmanager.h"

#include <QToolBar>
#include <QTextCursor>
#include <QDebug>

CommandLog::CommandLog(QWidget *parent)
    : EditWindow(parent, true)
{
    fileToolBar->removeAction(newAct);
    fileToolBar->removeAction(openAct);
    if (!isReadOnly)
    {
        editToolBar->removeAction(cutAct);
        editToolBar->removeAction(pasteAct);
    }
    historyIndex=0;
    tabIdx=0;
    lastWordCopy="";
    remainderCopy="";


    objectListFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    objectListFilter->setNoFilter();
    objectListFilter->setTypeList(QStringList() << "layer" << "connection"
                                  << "dataframe" << "representation"
                                  << "pattern" << "conversion" << "observer" << "nobserver"
                                  << "grouping" << "steps" << "xfile"
                                  );
//    objectListFilter->setType("<all objects>");

}

CommandLog::~CommandLog()
{

}

void CommandLog::addText(QString txt)
{
    history.append(txt);
    historyIndex = history.size();
    textEdit->moveCursor(QTextCursor::End);
    textEdit->appendPlainText(txt);
    textEdit->moveCursor(QTextCursor::End);
    currentLine = 1+textEdit->document()->blockCount();
}


//QString CommandLog::getHistory(int shift)
//{
//    QTextCursor *cursor = new QTextCursor(textEdit->document());
//    qDebug() << "Entered getHistory: line = " << currentLine << "pos = " << cursor->position();
//    QString line = "";
//    cursor->movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,currentLine-1);
//    cursor->movePosition(QTextCursor::StartOfLine);
//    qDebug() << "attempted move to start of current line: line = " << currentLine << "pos = " << cursor->position();
//    if (shift<0)
//    {
//        qDebug() << "Subtracting 1 from currentLine";
//        currentLine--;
//        if (currentLine==0)
//        {
//            qDebug() << "looping and returning to last line: line = " << currentLine;
//            currentLine = 1+textEdit->document()->blockCount();
//            return("");
//        }
//        else if (currentLine==textEdit->document()->blockCount())
//        {
//            qDebug() << "on last line; end, then up: line = " << currentLine << "pos = " << cursor->position();
//            cursor->movePosition(QTextCursor::EndOfLine);
//            cursor->movePosition(QTextCursor::Up, QTextCursor::KeepAnchor, abs(shift));
//        }
//        else
//        {
//            qDebug() << "attempting move up one line: line = " << currentLine << "pos = " << cursor->position();
//            cursor->movePosition(QTextCursor::Up, QTextCursor::KeepAnchor, abs(shift));
//            cursor->movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, abs(shift));
//        }
//        line=cursor->selectedText();
//        cursor->movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
//        qDebug() << "attempted move to start of line: line = " << currentLine << "pos = " << cursor->position();
//    }
//    if (shift>0) // cursor down
//    {
//        if (currentLine>0)
//            currentLine++;
//        if (currentLine==(1+textEdit->document()->blockCount()))
//        {
////            qDebug() << "on last line, just capturing the end: line = " << currentLine;
////            cursor->movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor, abs(shift));
//            qDebug() << "on last line, return blank: line = " << currentLine;
//            return("");

//        }
//        else if (currentLine>(textEdit->document()->blockCount()))
//        {
//            qDebug() << "looping round, grab first line: line = " << currentLine;
//            currentLine=1;
//            cursor->movePosition(QTextCursor::Start);
//            cursor->movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, abs(shift));
//        }
//        else
//        {
//            if (currentLine>0)
//                cursor->movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);
//            cursor->movePosition(QTextCursor::StartOfLine);
//            cursor->movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
//        }
//        qDebug() << "attempted move down one line: line = " << currentLine;
//        line=cursor->selectedText();
//    }
//    return(line.trimmed());
//}

QString CommandLog::getHistory(int shift, QString text)
{
    QString lastWord, remainder, newremainder;
    QStringList words;

    qDebug() << "Entered getHistory, tabIdx is " << tabIdx;
    qDebug() << "text is " << text;
    qDebug() << "shift is " << shift;
    if  (shift!=200) // not repeated TAB
        tabIdx=0;

    if (shift<0) // UP
    {
        if (history.count() && historyIndex)
            historyIndex--;
        return(history[historyIndex]);
    }
    else if (shift==1) //DOWN
    {
        if (history.count())
        {
            if (historyIndex < (history.size() - 1))
                historyIndex++;
            return(history[historyIndex]);
        }
    }
    else if (shift==0) //ESC
    {
        historyIndex = history.size();
        return("");
    }
    else if (shift==100 || shift==200) //TAB
    {
        words = text.split(" ");
        qDebug() << "word list is " << words;

        qDebug() << "tabIdx is " << tabIdx;
        qDebug() << "text is " << text;
        qDebug() << "remainderCopy is " << remainderCopy;
        if (tabIdx==0) // this is the first time user has pressed tab in this cycle
        {
            lastWord = words.back();
            words.removeLast();
            remainder = words.join(" ");
            if (remainder.size()) remainder += QString(" ");
            remainderCopy = remainder;
            lastWordCopy = lastWord;
        }
        else
        {
            lastWord = lastWordCopy;
            remainder = remainderCopy;
        }
        qDebug() << "last word is " << lastWord;
        qDebug() << "remainder is " << remainder;
        if (lastWord.size() == 0) return(text);
        QModelIndexList idxList = objectListFilter->match(objectListFilter->index(0, 0), Qt::DisplayRole, QVariant::fromValue(lastWord), -1, Qt::MatchStartsWith);
        qDebug() << "idxList is " << idxList;
        if (tabIdx >= idxList.size()) tabIdx = 0; // idxList.size() - 1;
        if (idxList.size())
            qDebug() << "name:" << objectListFilter->data(idxList.at(tabIdx), Qt::DisplayRole).toString();
        if (idxList.size())
            text = remainder.append(objectListFilter->data(idxList.at(tabIdx), Qt::DisplayRole).toString());
        tabIdx++;
        return(text);

    }

}
