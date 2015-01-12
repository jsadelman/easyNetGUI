#ifndef EDITWINDOW
#define EDITWINDOW

#include <QMainWindow>
#include "codeeditor.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
QT_END_NAMESPACE

class editWindow : public QMainWindow
{
    Q_OBJECT

public:
    editWindow(QWidget *parent = 0, QAction *p_newAct = NULL, QAction *p_openAct = NULL, bool cutAllowed=true, bool pasteAllowed=true);
    CodeEditor      *textEdit;
    bool maybeSave();
    void setCurrentFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void documentWasModified();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

//    QPlainTextEdit *textEdit;
    QString curFile;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *findAct;

    bool    cutAllowed, pasteAllowed;
};


#endif // EDITWINDOW

