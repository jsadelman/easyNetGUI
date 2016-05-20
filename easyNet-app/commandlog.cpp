#include "commandlog.h"
#include "codeeditor.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "objectcache.h"


#include "sessionmanager.h"

#include <QToolBar>
#include <QTextCursor>
#include <QDebug>

CommandLog::CommandLog(QWidget *parent, bool inclNewAct)
    : EditWindow(parent, true)
{
    if (!inclNewAct) fileToolBar->removeAction(newAct);
    fileToolBar->removeAction(openAct);
    if (!isReadOnly)
    {
        editToolBar->removeAction(cutAct);
        editToolBar->removeAction(pasteAct);
    }
    init();

    objectListFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    objectListFilter->setAllPassFilter();
//    objectListFilter->setTypeList(QStringList() << "layer" << "connection"
//                                  << "dataframe" << "representation"
//                                  << "pattern" << "conversion" << "observer" << "nobserver"
//                                  << "grouping" << "steps" << "xfile"
//                                  );

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

void CommandLog::init()
{
    history.clear();
    historyIndex=0;
    tabIdx=0;
    lastWordCopy="";
    remainderCopy="";
    currentLine = 1;

    cmdList = QStringList() << "list" << "of"
                            << "commands" << "here";
}


QString CommandLog::getHistory(int shift, QString text)
{
    QString lastWord, remainder, newremainder;
    QStringList words;
    int pos=0;
    static int oldPos=0;
    // remove leading spaces
    bool finalspace=false;
    if (text.right(1)==" ") finalspace=true;
    text = text.trimmed();
    if (finalspace) text.append(" ");

//    qDebug() << "Entered getHistory, tabIdx is " << tabIdx;
//    qDebug() << "text is " << text;
//    qDebug() << "shift is " << shift;

    if  (shift!=200) // not repeated TAB
        tabIdx=0;

    if (shift<0) // UP
    {
        if (history.count() && historyIndex)
        {
            historyIndex--;
            if (historyIndex >= history.size())
                historyIndex = history.size() - 1;
            return(history[historyIndex]);
        }
        return("");
    }
    else if (shift==1) //DOWN
    {
        if (history.count())
        {
            if (historyIndex < (history.size() - 1))
                historyIndex++;
            if (historyIndex >= history.size())
                historyIndex = history.size() - 1;
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
//        qDebug() << "word list is " << words;
        int numSpaces = text.count(QLatin1Char(' '));
//        qDebug() << "numSpaces is " << numSpaces;
//        qDebug() << "words.size() is " << words.size();
        if (numSpaces==0)
            pos=1;
        else if ((numSpaces>=1) && (words.size()>=1) && (words.size()<3))
            pos=2;
        if (shift==200)
            pos=oldPos; // tabbing can't change pos -- this is to sidestep pos change caused by tab completion introducing spaces
        oldPos=pos;
//        qDebug() << "tabIdx is " << tabIdx;
//        qDebug() << "text is " << text;
//        qDebug() << "remainderCopy is " << remainderCopy;
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
//        qDebug() << "last word is " << lastWord;
//        qDebug() << "remainder is " << remainder;
//        qDebug() << "pos is " << pos;
        if (lastWord.size() == 0) return(text);
        QStringList keywordList;
        if (pos!=2)
        {
//            qDebug() << "searching for" << lastWord;
//            qDebug() << "searching for QVariant" << QVariant::fromValue(lastWord);
//            qDebug() << objectListFilter->index(0, 0);
            QModelIndexList idxList = objectListFilter->match(objectListFilter->index(0, 0), Qt::DisplayRole, QVariant::fromValue(lastWord), -1, Qt::MatchStartsWith | Qt::MatchCaseSensitive);
            QString searchTerm=QString("^.*$");
            if (lastWord.size()) searchTerm = QString("^") + lastWord + QString(".+");

            if (pos==1) // only print from keywordList when in pos 1
                keywordList = SessionManager::instance()->lazyNutkeywords.filter(QRegExp(searchTerm));
            else
                keywordList = QStringList();
//            qDebug() << "idxList is " << idxList;
//            qDebug() << "searchTerm is " << searchTerm;
            if (tabIdx >= idxList.size() + keywordList.size()) tabIdx = 0; // idxList.size() - 1;
            if (tabIdx < idxList.size())
            {
//                qDebug() << "object name:" << objectListFilter->data(idxList.at(tabIdx), Qt::DisplayRole).toString();
                text = remainder.append(objectListFilter->data(idxList.at(tabIdx), Qt::DisplayRole).toString());
            }
            else
            {
//                qDebug() << "idxList.size()" << idxList.size();
//                qDebug() << "keywordList" << keywordList;
                int k_idx = tabIdx-idxList.size();
                if (k_idx>0)
                {
//                    qDebug() << "command name:" << keywordList.at(k_idx);
                    text = remainder.append(keywordList.at(k_idx));
                }
             }
        }
        else
        {
            if (tabIdx >= cmdList.size())
                tabIdx = 0;
            else
                text=remainder.append(cmdList.at(tabIdx));
        }
        tabIdx++;

        return(text);

    }
    return("");

}


void CommandLog::clear()
{
    EditWindow::clear();
    init();
}
