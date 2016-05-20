#ifndef COMMANDLOG_H
#define COMMANDLOG_H

#include "editwindow.h"
#include "sessionmanager.h"


QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class CodeEditor;
class MainWindow;
class ObjectCacheFilter;
class ObjectUpdater;

QT_END_NAMESPACE

class CommandLog : public EditWindow
{
    Q_OBJECT
    friend class MainWindow;

public:
    CommandLog(QWidget *parent = 0, bool inclNewAct = false);
    ~CommandLog();
public slots:
    QString getHistory(int shift, QString text);
    virtual void clear();
    void addText(QString txt);
private:
    void init();

    int currentLine;
    int historyIndex;
    int tabIdx;
    QStringList history;
    QString lastWordCopy;
    QString remainderCopy;
    ObjectCacheFilter *objectListFilter;
    QStringList cmdList;

};

#endif // COMMANDLOG_H
