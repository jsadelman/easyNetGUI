#ifndef COMMANDLOG_H
#define COMMANDLOG_H

#include "editwindow.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class CodeEditor;
class EasyNetMainWindow;
QT_END_NAMESPACE

class CommandLog : public EditWindow
{
    Q_OBJECT
    friend class EasyNetMainWindow;

public:
    CommandLog(QWidget *parent = 0);
    ~CommandLog();
public slots:
    QString getHistory(int shift);

    void addText(QString txt);
private:
    int currentLine;

};

#endif // COMMANDLOG_H
