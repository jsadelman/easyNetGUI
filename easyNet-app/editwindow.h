#ifndef EDITWINDOW
#define EDITWINDOW

#include <QMainWindow>
#include <QTextDocument>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QPlainTextEdit;
class CodeEditor;
class MainWindow;
class FindDialog;

class QFlag;
QT_END_NAMESPACE

class EditWindow : public QMainWindow
{
    Q_OBJECT
    friend class MainWindow;
public:
    EditWindow(QWidget *parent = 0, bool isReadOnly = false);
    CodeEditor      *textEdit;
//    QPlainTextEdit      *textEdit;
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
    QAction *stopScriptAct;
    QAction *pauseAct;
    QString startDir;


    bool maybeSave();
    void setCurrentFile(const QString &fileName);

public slots:
    void addText(QString txt);

    void findForward(const QString &str, QFlags<QTextDocument::FindFlag> flags);
    void findBackward(const QString &str, QFlags<QTextDocument::FindFlag> flags );
    void setFilenameLabel(QString filename);
    virtual void clear();
signals:
    void runCmdRequested(QStringList);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

protected slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void documentWasModified();
    void showFindDialog();

protected:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    QLabel *filenameLabel;

//    QPlainTextEdit *textEdit;
    QString curFile;

    QMenu *editMenu;
    QMenu *helpMenu;
    FindDialog* findDialog;

    bool isReadOnly;
    //bool    cutAllowed, pasteAllowed;
};


#endif // EDITWINDOW

