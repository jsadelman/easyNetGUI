#ifndef NMCONSOLE_H
#define NMCONSOLE_H

#include <QDialog>
#include <QMainWindow>
#include <QLineEdit>
#include <QProcess>
//#include <QDockWidget>
#include <QPlainTextEdit>
#include <QRegExp>
#include <QSplitter>
#include <QTreeView>
#include <QHash>
#include <QPixmap>
#include <QWebView>
#include <QLabel>
#include <QGraphicsSvgItem>
#include <QtSvg>


//#include "driver.h"
//#include "parsenode.h"
//#include "querycontext.h"
//#include "treemodel.h"


#include "highlighter.h"
#include "codeeditor.h"
#include "editwindow.h"
#include "plotwindow.h"


class QGroupBox;
class QDockWidget;

QT_BEGIN_NAMESPACE
class QueryContext;
class TreeModel;
class TreeItem;
class LazyNutObj;
typedef QHash<QString,LazyNutObj*> LazyNutObjCatalogue;
class ObjExplorer;
namespace lazyNutOutputParser {
    class Driver;
}
class DesignWindow;
class LazyNut;
class CommandSequencer;
class SessionManager;

QT_END_NAMESPACE

class InputCmdLine : public QLineEdit
{
    Q_OBJECT

public:
    InputCmdLine(QWidget *parent = 0);

public slots:
    void sendCommand();

signals:
    void commandReady(const QString & command);

};

class CmdOutput : public QPlainTextEdit
{
    Q_OBJECT

public:
    CmdOutput(QWidget *parent = 0);

public slots:
    void displayOutput(const QString & output);

};

class LazyNutScriptEditor: public QPlainTextEdit
{
    Q_OBJECT

public:
    LazyNutScriptEditor(QWidget *parent = 0);

public slots:
    QStringList getSelectedText();
    QStringList getAllText();

};

//class NM : public QProcess
//{
//    Q_OBJECT

//  public:
//    NM(QObject *parent = 0);
//    ~NM();

//  signals:
//    void outputReady(const QString & line);

//  public slots:
//    void sendCommand(const QString & line);
//    void getNMError();

//};


//class QueryProcessor : public QSplitter
//{
//    Q_OBJECT

//public:
//    QueryProcessor(LazyNutObjCatalogue* objHash, TreeModel* objTaxonomyModel,  QWidget *parent=0);
//    void testDesignWindow();

//signals:
//  void treeReady(const QString & tree);
//  void commandReady(const QString & output);
//  void resultAvailable(QString);
//  void beginObjHashModified();
//  void endObjHashModified();

//public slots:
//    void getTree(const QString & query);

//private:
//    void processQueries();

//    QueryContext* context;
//    lazyNutOutput::Driver* driver;
//    LazyNutObjCatalogue* objHash;
//    TreeModel* objTaxonomyModel;
//    int contextQuerySize;
//    QString lazyNutBuffer;
//    QString treeOutput;
//    QRegExp rxEND;
//    CmdOutput *parseOutput;


//};


//class NmCmd : public QWidget
//{
//    // this class should be removed.
//    // A QGroupBox is enough.
//    Q_OBJECT

//public:
//    NmCmd(QWidget *parent = 0);

//    CmdOutput       *cmdOutput;
//    InputCmdLine       *inputCmdLine;


//};

class NmConsole : public QMainWindow
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
    NmConsole(QWidget *parent = 0);

signals:
    void savedLayoutToBeLoaded(QString);
    void saveLayout();
    void viewModeClicked(int);

private slots:
    void showViewMode(int viewModeInt);
//    void showWelcomeView();
//    void showCodeView();
//    void showModelView();
//    void showTrialView();
//    void showInputView();
//    void showOutputView();
//    void showParameterView();
//    void showInterpreterView();
    void newScriptFile();
    void newLogFile();
    void open();
    void run();
    //bool save();
    //bool saveAs();
    //void documentWasModified();

    void echoCommand(const QString &line);
//    void runScript();
    void runModel();
    void runSelection();
    void runCmd(QString cmd);
    void setEasyNetHome();
    void setLazyNutBat();
    void showPauseState(bool isPaused);

    void lazyNutNotRunning();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void initViewActions();
    void createViewActions();
    void createActions();
    void createMenus();
    void createToolBars();
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
#elif defined(_WIN32)
    QString         lazyNutExt = "bat";
    QString         binDir = "bin";
#endif
    QString         lazyNutBasename = QString("lazyNut.%1").arg(lazyNutExt);
    QString         curFile;
    QString         curJson;
    QString         scriptsDir;
    QString         easyNetHome = "";
    QStringList lazyNutObjTypes{"layer","connection","conversion","representation","pattern","steps","database","file","observer"};


    //NmCmd           *nmCmd;
    QGroupBox       *lazyNutInterpreter;
    CmdOutput       *cmdOutput;
    InputCmdLine    *inputCmdLine;
    //NM              *lazyNut;
    LazyNut         *lazyNut;
    CommandSequencer    *commandSequencer;
    SessionManager      *sessionManager;
    TreeModel       *objTaxonomyModel;
    LazyNutObjCatalogue  *objCatalogue;
//    QueryProcessor   *queryProcessor;
    ObjExplorer      *objExplorer;
    QLabel          *zebPic;

    QDockWidget     *dockZeb;
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
    PlotWindow      *plotForm;
    QToolBar        *infoToolBar;
//    QVBoxLayout     *vLayout;

    QSignalMapper   *viewModeSignalMapper;
    QList<QToolButton*> viewModeButtons;
    QMenu           *fileMenu;
    QMenu           *runMenu;
    QMenu           *settingsMenu;
//    QToolBar        *fileToolBar;
//    QToolBar        *runToolBar;

    QList<QIcon*>   viewActionIcons;
    QStringList     viewActionTexts;
    QStringList     viewActionStatusTips;
    QList<QAction*> viewActions;

//    QAction         *welcomeAction;
//    QAction         *viewModelAction;
//    QAction         *viewCodeAction;
//    QAction         *viewInterpreterAction;
//    QAction         *viewTrialAction;
//    QAction         *viewInputAction;
//    QAction         *viewOutputAction;
//    QAction         *viewParamsAction;
    QAction         *newScriptAct;
    QAction         *newLogAct;
    QAction         *openAct;
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
//    QAction         *synchModeAct;
};

#endif // NMCONSOLE_H
