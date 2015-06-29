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
class QDomDocument;

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
//class LazyNutScriptEditor;
class MaxMinPanel;
class TableEditor;
class LazyNutConsole;
class Assistant;
class TextEdit;
class HelpWindow;

QT_END_NAMESPACE

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
    void save();
    void about();
    void updateTableView(QString text);

//    void showViewMode(int viewModeInt);
    void newScriptFile();
    void newLogFile();
    void open();
    void setLazyNutIsReady(bool isReady);
    //bool save();
    //bool saveAs();
    //void documentWasModified();
    void loadModel();
    void loadStimulusSet();
    void currentStimulusChanged(QString stim);
    void msgBox(QString msg);
    void runTrial();
    void runAllTrial();

    void showErrorOnStatusBar(QString  /*cmd*/, QStringList errorList);
    void clearErrorOnStatusBar();
    void showCmdOnStatusBar(QString cmd);
    void addOneToLazyNutProgressBar();

    void echoCommand(const QString &line);
    void runScript();
    void runSelection();
    void runCmdAndUpdate(QStringList cmdList);
    void setEasyNetHome();
    void setLazyNutBat();
    void showPauseState(bool isPaused);
    void getVersion();
//    void showDocumentation();

    void lazyNutNotRunning();
//    void requestVersion();
    void displayVersion(QString version);

    void updateStimuliView(QString text);
    void showDocumentation();
protected:
    void closeEvent(QCloseEvent *event);

private:
    void checkLazyNut();
    void constructForms();
    void checkScreens();
    void createDockWindows();
    void initialiseToolBar();
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

    QTextEdit *textEdit1;
    QTextEdit *textEdit2;
    QTextEdit *textEdit3;
    QTextEdit *textEdit4;
    QTextEdit *textEdit5;
    QTextEdit *textEdit12;
    QTabWidget *centrePanel;
    QTabWidget *leftPanel;
    QTabWidget *lowerRightPanel;
    QTabWidget *upperRightPanel;
    MaxMinPanel* codePanelDock;

    QListWidget* customerList;
    QListWidget* paragraphsList;

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
    TableEditor     *stimSetForm;
    TableEditor     *tablesWindow;
    QToolBar        *infoToolBar;
    QToolBar        *toolbar;
    QComboBox       *modelComboBox;
    QComboBox       *trialComboBox;
    QComboBox       *setComboBox;
    QComboBox       *inputComboBox;
    QWidget         *spacer;
    ObjectCatalogueFilter* modelListFilter;
    ObjectCatalogueFilter* trialListFilter;
    Assistant       *assistant;
//    TextEdit        *textViewer;
    HelpWindow      *infoWindow;

    QSignalMapper   *viewModeSignalMapper;
    QList<QToolButton*> viewModeButtons;
    QMenu           *fileMenu;
    QMenu           *runMenu;
    QMenu           *settingsMenu;
    QMenu           *aboutMenu;
    QMenu           *editMenu;
    QMenu           *viewMenu;
    QMenu           *helpMenu;


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
//    QAction         *runAction;
    QAction         *runScriptAct;
    QAction         *runSelectionAct;
    QAction         *stopAct;
    QAction         *pauseAct;
    QAction         *setEasyNetHomeAct;
    QAction         *setLazyNutBatAct;
    QAction         *versionAct;
    QAction         *assistantAct;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *saveAct;
    QAction *printAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *quitAct;

    bool trialComboEventSwitch = false;
    bool test_gui;

};

#endif // NMCONSOLE_H
