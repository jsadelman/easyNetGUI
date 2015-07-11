#ifndef EDITWINDOW
#define EDITWINDOW

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class CodeEditor;
class EasyNetMainWindow;
QT_END_NAMESPACE

class EditWindow : public QMainWindow
{
    Q_OBJECT
    friend class EasyNetMainWindow;
public:
    EditWindow(QWidget *parent = 0, QAction *p_newAct = NULL, QAction *p_openAct = NULL, bool isReadOnly = false);
    CodeEditor      *textEdit;
    QMenu *fileMenu;
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
    QAction *stopAct;
    QAction *pauseAct;

    bool maybeSave();
    void setCurrentFile(const QString &fileName);

public slots:
    void addText(QString txt);

signals:
    void runCmdAndUpdate(QStringList);

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

    QMenu *editMenu;
    QMenu *helpMenu;

    bool isReadOnly;
    //bool    cutAllowed, pasteAllowed;
};


#endif // EDITWINDOW

