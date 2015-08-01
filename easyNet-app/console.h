#ifndef CONSOLE_H
#define CONSOLE_H

#include "editwindow.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class CodeEditor;
class EasyNetMainWindow;
class InputCmdLine;
QT_END_NAMESPACE

class Console : public EditWindow
{
    Q_OBJECT
    friend class EasyNetMainWindow;

public:
    Console(QWidget *parent = 0);
    ~Console();
public slots:
    void setConsoleFontSize(int size);
    void addText(QString txt);
signals:
    void historyKey(int dir);
private slots:
    void showHistory(QString line);
private:
    int currentLine;
    InputCmdLine* inputCmdLine;


    void createStatusBar();
};

#endif // CONSOLE_H
