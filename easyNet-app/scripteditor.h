#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "editwindow.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class CodeEditor;
class EasyNetMainWindow;
QT_END_NAMESPACE

class ScriptEditor : public EditWindow
{
    Q_OBJECT
    friend class EasyNetMainWindow;

public:
    ScriptEditor(QString _startDir, QWidget *parent = 0);
    ~ScriptEditor();

    QToolBar *runToolBar;
    QAction *runAct;
    QAction *runSelectionAct;

public slots:
    void runScript();
    void runSelection();
private:

};

#endif // SCRIPTEDITOR_H
