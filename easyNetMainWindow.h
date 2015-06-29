#ifndef NMCONSOLE_H
#define NMCONSOLE_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QtWidgets>

QT_BEGIN_NAMESPACE
class QLabel;
class QWebView;
class QGroupBox;
class QDockWidget;
class QSignalMapper;
class QStackedWidget;
class QProgressBar;

class ObjExplorer;

class DesignWindow;
class LazyNut;
class SessionManager;
class AsLazyNutObject;
typedef QHash<QString,AsLazyNutObject*> LazyNutObjectCatalogue;
class ObjectCatalogue;
class EditWindow;
class Highlighter;
class PlotWindow;
class ObjectCatalogueFilter;
class LazyNutListComboBox;
class LazyNutConsole;

QT_END_NAMESPACE




class LazyNutScriptEditor: public QPlainTextEdit
{
    Q_OBJECT

public:
    LazyNutScriptEditor(QWidget *parent = 0);

public slots:
    QStringList getSelectedText();
    QStringList getAllText();

};


class EasyNetMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum ViewMode
    {
        ViewMode_BEGIN,
        Welcome = ViewMode_BEGIN,
        Model,
        Trial,
        Input,
        Output,
        Params,
        Interpreter,
        Code,
        ViewMode_END
    };
    EasyNetMainWindow(QWidget *parent = 0);
//    SessionManager *sessionManager;

signals:
    void savedLayoutToBeLoaded(QString);
    void saveLayout();
    void viewModeClicked(int);

private slots:
    void showViewMode(int viewModeInt);
    void newScriptFile();
    void newLogFile();
    void open();
    void run();
    void setLazyNutIsReady(bool isReady);
    //bool save();
    //bool saveAs();
    //void documentWasModified();
    void loadModel();
//    void loadStimulusSet();
    void msgBox(QString msg);
    void runTrial();
    void runAllTrial();

    void showErrorOnStatusBar(QString  /*cmd*/, QStringList errorList);
    void clearErrorOnStatusBar();
    void showCmdOnStatusBar(QString cmd);
    void addOneToLazyNutProgressBar();

    void echoCommand(const QString &line);
//    void runScript();
    void runModel();
    void runSelection();
    void runCmdAndUpdate(QStringList cmdList);
    void setEasyNetHome();
    void setLazyNutBat();
    void showPauseState(bool isPaused);
    void getVersion();

    void lazyNutNotRunning();
//    void requestVersion();
    void displayVersion(QString version);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void checkScreens();
    void initialiseToolBar();
    void updateToolBar();

    void initViewActions();
    void createViewActions();
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
//    bool maybeSave();
    void newFile(EditWindow*);
    void loadFile(const QString &fileName);
//    bool saveFile(const QString &fileName);

    void setCurrentFile(EditWindow *window, const QString &fileName);
    QString strippedName(const QString &fullFileName);
    void readSettings();
    void writeSettings();
//    void runLazyNutBat();

    void hideAllDocks();


    QString         lazyNutBat= "";
//    QString         curFile;
#if defined(__linux__)
    QString         lazyNutExt = "sh";
    QString         binDir = "bin-linux";
#elif defined(__APPLE__)
    QString         lazyNutExt = "sh";
    QString         binDir = "bin-mac";
#elif defined(_WIN32)
    QString         lazyNutExt = "bat";
    QString         binDir = "bin";
#endif
    QString         lazyNutBasename = QString("lazyNut.%1").arg(lazyNutExt);
    QString         curFile;
    QString         curJson;
    QString         scriptsDir;
    QString         stimDir;
    QString         easyNetHome = "";

    QStringList     modelList;
    QStringList     trialList;

//    TreeModel       *objTaxonomyModel;
//    LazyNutObjCatalogue  *objCatalogue;
    LazyNutConsole *lazyNutConsole;
    ObjExplorer      *objExplorer;

    QDockWidget     *dockWelcome;
    QDockWidget     *dockWebWelcome;
    QDockWidget     *dockEdit;
    QDockWidget     *dockCommandLog;
    QDockWidget     *dockInterpreter;
    QDockWidget     *dockExplorer;
    QDockWidget     *dockDesignWindow;
    QDockWidget     *dockInput;
    QDockWidget     *dockOutput;

    QWebView        *welcomeScreen;
    QWebView        *webWelcomeScreen;
//    CodeEditor      *scriptEdit;
    EditWindow       *scriptEdit;
//    CodeEditor      *commandLog;
    EditWindow       *commandLog;
    Highlighter     *highlighter;
    Highlighter     *highlighter2;
//    LazyNutScriptEditor  *scriptEditor;
    DesignWindow    *designWindow;
    PlotWindow      *plotWindow;
    QToolBar        *infoToolBar;
    QToolBar        *toolbar;
    QComboBox       *modelComboBox;
    QComboBox       *trialComboBox;
    QComboBox       *setComboBox;
    QComboBox       *inputComboBox;
    QWidget         *spacer;
    ObjectCatalogueFilter* modelListFilter;
    ObjectCatalogueFilter* trialListFilter;

    QSignalMapper   *viewModeSignalMapper;
    QList<QToolButton*> viewModeButtons;
    QMenu           *fileMenu;
    QMenu           *runMenu;
    QMenu           *settingsMenu;
    QMenu           *aboutMenu;

    // status bar widgets
    QLabel          *readyLabel;
    QLabel          *busyLabel;
    QLabel          *offLabel;
    QStackedWidget  *lazyNutStatusWidget;
    QProgressBar    *lazyNutProgressBar;
    QLabel          *lazyNutCmdLabel;
    QLabel          *lazyNutErrorLabel;


//    QToolBar        *fileToolBar;
//    QToolBar        *runToolBar;

    QList<QIcon*>   viewActionIcons;
    QStringList     viewActionTexts;
    QStringList     viewActionStatusTips;
    QList<QAction*> viewActions;

    QAction         *newScriptAct;
    QAction         *newLogAct;
    QAction         *openAct;
    QAction         *loadModelAct;
    QAction         *loadStimulusSetAct;
//    QAction         *saveAct;
//    QAction         *saveAsAct;
    QAction         *exitAct;
    QAction         *runAction;
    QAction         *runScriptAct;
    QAction         *runSelectionAct;
    QAction         *stopAct;
    QAction         *pauseAct;
    QAction         *setEasyNetHomeAct;
    QAction         *setLazyNutBatAct;
    QAction         *versionAct;

    bool                trialComboEventSwitch = false;
};

#endif // NMCONSOLE_H
