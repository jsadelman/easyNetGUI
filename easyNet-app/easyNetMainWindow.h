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

class PlotViewer;
class PlotViewerDispatcher;
class ObjExplorer;
class DesignWindow;
class LazyNut;
class SessionManager;
class AsLazyNutObject;
typedef QHash<QString,AsLazyNutObject*> LazyNutObjectCatalogue;
class ObjectCache;
class EditWindow;
class CommandLog;
class ScriptEditor;
class Highlighter;
//class PlotSettingsWindow;
class ObjectCacheFilter;
//class LazyNutListComboBox;
//class LazyNutScriptEditor;
class MaxMinPanel;
class TableEditor;
class TableViewer;
class Assistant;
class TextEdit;
class HelpWindow;
class TrialWidget;
class TrialEditor;
class ModelSettingsDisplay;
class Console;
class DebugLog;
class DiagramSceneTabWidget;
class DiagramScene;
class DiagramWindow;
//class TableWindow;
class DataframeViewer;
class DataframeViewerDispatcher;
class Ui_DataTabsViewer;
class Ui_DataComboViewer;
class QTime;
class SettingsWidget;
class FloatingDialogWindow;

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(bool debugMode READ debugMode WRITE setDebugMode NOTIFY debugModeChanged)
    Q_PROPERTY(int trialListLength READ trialListLength WRITE setTrialListLength)
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
    static MainWindow* instance(); // singleton

//    SessionManager *sessionManager;

public:
    void build();
    bool debugMode() {return m_debugMode;}
    int trialListLength() {return m_trialListLength;}
    void setTrialListLength(int length) {m_trialListLength = length;}

public slots:
    void loadTrial();
    void loadAddOn();
    void showPlotViewer();
    void importDataFrame();
    void msgBox(QString msg);
    void setDebugMode(bool isDebugMode);
    void updateTrialRunListCount(int count);
    void initialLayout();
    void showModelSettings();
    void showParameterSettings();
    void setStopButtonIcon(bool state);
signals:
    void savedLayoutToBeLoaded(QString);
    void saveLayout();
    void viewModeClicked(int);
    void paramTabEntered(QString);
    void newTableSelection(QString name);
    void showHistory(QString line);
    void runAllTrialEnded();
    void debugModeChanged(bool);

private slots:
//    void about();
//    void updateTableView(QString text);

//    void showViewMode(int viewModeInt);
    void newScriptFile();
    void newLogFile();
    void loadScript();
    void setLazyNutIsReady(bool isReady);
    //bool save();
    //bool saveAs();
    //void documentWasModified();
    void loadModel();
    void loadModelUnconfigured();
    void loadStimulusSet();
    void currentStimulusChanged(QString stim);

//    void runTrial();
//    void runSingleTrial();
//    void runTrialList();

    void showErrorOnStatusBar(QString  /*cmd*/, QStringList errorList);
    void clearErrorOnStatusBar();
    void showCmdOnStatusBar(QString cmd);
    void addOneToLazyNutProgressBar();


    void viewSettings();
    void setNewEasyNetHome();
    void setNewEasyNetDataHome();

//    void showPauseState(bool isPaused);
    void getVersion();
//    void showDocumentation();

    void lazyNutNotRunning();
    void setLazyNutFinished(bool crashed);
//    void requestVersion();
    void displayVersion(QString version);

    void showDocumentation();
    void setParam(QString paramDataFrame, QString newParamValue);


    void setFontSize(const QString &size);
    void modelConfigNeeded();
    void createModelSettingsDialog(QDomDocument* domDoc);
    void afterModelConfig();
    void afterModelStaged();
    void diagramSceneTabChanged(int index);
    void runScript();
    void processHistoryKey(int dir, QString text);
    void setQuietMode(bool state);
    void showExplorer();
    void restart();
    void showMostRecentError();
//    void showPlotSettings();
    void showDataViewSettings();
    void updateDFComboBox();
    void runTest();
    void afterTestsCompleted();
    void setFormInSettingsWidget(QString name);
    void switchFormInSettingsWidget(bool visible);
    void switchFormInSettingsWidget(QTabWidget *panel=nullptr);
    void showResultsViewer(QString name);

protected:
    void closeEvent(QCloseEvent *event);

private:
    MainWindow(QWidget *parent = 0);
    MainWindow(MainWindow const&){}
    MainWindow& operator=(MainWindow const&){}
    static MainWindow* mainWindow;

    void setNewEasyNetDir(QString env);
    void constructForms();
    void checkScreens();
    void createDockWindows();
    void initialiseToolBar();
    void createActions();
    void createMenus();
    void createStatusBar();
    void connectSignalsAndSlots();
//    bool maybeSave();
    void newFile(EditWindow*);
    void loadFile(const QString &fileName);
//    bool saveFile(const QString &fileName);

    void setCurrentFile(EditWindow *window, const QString &fileName);
    void readSettings();
    void writeSettings();
//    void runLazyNutBat();

    void hideAllDocks();
    bool proceedWithRestartOk();
    //void loadModel(QString fileName);
    //void loadModelUnconfigured(QString fileName);
    void loadModel(QString fileName,bool complete);

    bool m_debugMode;
public:


    QString         curFile;
    QString         paramDataFrame;
    QString         quietMode;
    QString         df_name_for_updating_combobox;

    QStringList     modelList;
    QStringList     trialList;

//    QTextEdit *textEdit12;
    QTabWidget *methodsPanel;
    QTabWidget *introPanel;
    QTabWidget *lazynutPanel;
    QTabWidget *resultsPanel;
    QTabWidget *explorerPanel;
    DiagramSceneTabWidget *diagramPanel;
    DiagramWindow *diagramWindow;
//    QDockWidget* introDock;
    QDockWidget* codePanelDock;
    QDockWidget *methodsDock;
    QDockWidget *diagramDock;
    QDockWidget *explorerDock;
    QDockWidget *resultsDock;
    FloatingDialogWindow* modelSettingsDialog;
    FloatingDialogWindow* paramSettingsDialog;
    FloatingDialogWindow* dataViewSettingsDialog;

    QListWidget* customerList;
    QListWidget* paragraphsList;

    Console        *lazyNutConsole;
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
    ScriptEditor       *scriptEdit;
//    CodeEditor      *commandLog;
//    EditWindow       *commandLog;
    CommandLog       *commandLog;
    CommandLog       *errorLog;
    CommandLog       *rLog;
    Highlighter     *highlighter;
    Highlighter     *highlighter2;
    Highlighter     *highlighter3;
//    LazyNutScriptEditor  *scriptEditor;
//    DesignWindow    *designWindow;
//    DesignWindow    *conversionWindow;
    DiagramScene    *modelScene;
    DiagramScene    *conversionScene;
    QWidget         *conversionPage;
//    PlotSettingsWindow      *plotSettingsWindow;
    SettingsWidget      *dataViewSettingsWidget;
//    PlotViewer_old      *plotViewer;
    Ui_DataTabsViewer *ui_plotViewer;
    PlotViewerDispatcher *plotViewerDispatcher;
    PlotViewer      *plotViewer;
//    TableEditor     *stimSetForm;
    DataframeViewer *stimSetViewer;
    Ui_DataTabsViewer *ui_stimSetViewer;
//    TableEditor     *dataframesWindow;
    DataframeViewer *dataframeViewer;
    Ui_DataComboViewer *ui_dataframeViewer;
    DataframeViewer *testViewer;
    Ui_DataTabsViewer *ui_testViewer;

//    TableViewer     *tableWindow;
//    TableWindow     *tableWindow;
    DataframeViewer *dataframeResultsViewer;
    DataframeViewerDispatcher *dataframeResultsDispatcher;
    Ui_DataTabsViewer *ui_dataframeResultsViewer;


//    TableEditor     *paramEdit;
    DataframeViewer *paramViewer;
    Ui_DataTabsViewer *ui_paramViewer;
    DebugLog        *debugLog;
    QToolBar        *infoToolBar;
    QToolBar        *toolbar;
    QComboBox       *modelComboBox;
    QComboBox       *trialComboBox;
    TrialWidget     *trialWidget;
    TrialEditor     *trialEditor;
    ModelSettingsDisplay *modelSettingsDisplay;
    QDialog *settingsDialog;


    QPushButton     * trialButton;
    QPushButton     * modelButton;
    QToolButton     * addonButton;
    QToolButton     * stopButton;

    QWidget         *spacer;
    QAction         *stopAct;
    ObjectCacheFilter* modelListFilter;
    ObjectCacheFilter* trialListFilter;
    ObjectCacheFilter *dataframeDescriptionFilter;
    ObjectCacheFilter *paramDescriptionFilter;
    ObjectCacheFilter *testFilter;
    QAction         *runAllTrialMsgAct;
    int              m_trialListLength;
    int              trialListCount;

    Assistant       *assistant;
//    TextEdit        *textViewer;
    HelpWindow      *infoWindow;
    int             stimSetTabIdx;
    int             infoTabIdx;
    int             paramTabIdx;
    int             plotTabIdx;
    int             plotSettingsTabIdx;
    int             dataViewSettingsTabIdx;
    int             modelTabIdx;
    int             conversionTabIdx;
    int             scriptTabIdx;
    int             outputTablesTabIdx;
    int             trialFormTabIdx;
    int             modelSettingsTabIdx;
    int             dfTabIdx;
    int             explorerTabIdx;
    int             testsTabIdx;

    QSignalMapper   *viewModeSignalMapper;
    QSignalMapper   *setFontSignalMapper;
    QList<QToolButton*> viewModeButtons;
    QMenu           *fileMenu;
    QMenu           *fileSubMenu;
    QMenu           *settingsMenu;
    QMenu           *settingsSubMenu;
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
    QComboBox       *lazyNutErrorBox;
    QLabel          *runAllTrialLabel;


//    QToolBar        *fileToolBar;
//    QToolBar        *runToolBar;

    QList<QIcon*>   viewActionIcons;
    QStringList     viewActionTexts;
    QStringList     viewActionStatusTips;
    QList<QAction*> viewActions;

    QAction         *newScriptAct;
    QAction         *newLogAct;
    QAction         *loadScriptAct;
    QAction         *loadModelAct;
    QAction         *loadModelUAct;
    QAction         *modelFinalizeAct;
    QAction         *loadTrialAct;
    QAction         *loadAddOnAct;
    QAction         *loadStimulusSetAct;
    QAction         *importDataFrameAct;
    QAction         *runTestAct;
//    QAction         *saveAct;
//    QAction         *saveAsAct;
    QAction         *exitAct;
    QAction         *restartInterpreterAct;
    QAction         *viewSettingsAct;
    QActionGroup    *setFontActGrouop;
    QAction         *setSmallFontAct;
    QAction         *setMediumFontAct;
    QAction         *setLargeFontAct;
    QAction         *versionAct;
    QAction         *assistantAct;
    QAction         *setQuietModeAct;
    QAction         *debugModeAct;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *saveAct;
    QAction *printAct;
    QAction *aboutAct;
    QAction *quitAct;

    bool trialComboEventSwitch = false;
    QTime loadModelTimer;


//    enum runMode {RunSingle, RunAll};



};

#endif // NMCONSOLE_H
