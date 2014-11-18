#ifndef NMCONSOLE_H
#define NMCONSOLE_H

#include <QDialog>
#include <QMainWindow>
#include <QLineEdit>
#include <QProcess>
#include <QDockWidget>
#include <QPlainTextEdit>
#include <QRegExp>
#include <QSplitter>
#include <QTreeView>
#include <QHash>


//#include "driver.h"
//#include "parsenode.h"
//#include "querycontext.h"
//#include "treemodel.h"




QT_BEGIN_NAMESPACE
class QueryContext;
class TreeModel;
class TreeItem;
class LazyNutObj;
typedef QHash<QString,LazyNutObj*> LazyNutObjCatalogue;
class ObjExplorer;
namespace lazyNutOutput {
    class Driver;
}
class DesignWindow;
class LazyNut;
class CommandSequencer;
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

class NM : public QProcess
{
    Q_OBJECT

  public:
    NM(QObject *parent = 0);
    ~NM();

  signals:
    void outputReady(const QString & line);

  public slots:
    void sendCommand(const QString & line);
    void getNMError();

};

/*class ParseTree : public QTreeView //QSplitter
{
    Q_OBJECT

public:
    ParseTree(QWidget *parent = 0);
//    ~ParseTree();

signals:
  void treeReady(const QString & tree);

public slots:
    void getTree(const QString & query);

private:
    QueryContext* contextPtr;
    lazyNutOutput::Driver* driverPtr;
    int contextQuerySize;
    QString lazyNutBuffer;
    QString treeOutput;
    QRegExp rxEND;
    TreeModel treeModel;
    //QTreeView *treeView;
    CmdOutput *parseOutput;

};
*/
class QueryProcessor : public QSplitter
{
    Q_OBJECT

public:
    QueryProcessor(LazyNutObjCatalogue* objHash, TreeModel* objTaxonomyModel,  QWidget *parent=0);
    void testDesignWindow();

signals:
  void treeReady(const QString & tree);
  void commandReady(const QString & output);
  void resultAvailable(QString);
  void beginObjHashModified();
  void endObjHashModified();

public slots:
    void getTree(const QString & query);

private:
    void processQueries();

    QueryContext* context;
    lazyNutOutput::Driver* driver;
    LazyNutObjCatalogue* objHash;
    TreeModel* objTaxonomyModel;
    int contextQuerySize;
    QString lazyNutBuffer;
    QString treeOutput;
    QRegExp rxEND;
    CmdOutput *parseOutput;


};


class NmCmd : public QWidget
{
    // this class should be removed.
    // A QGroupBox is enough.
    Q_OBJECT

public:
    NmCmd(QWidget *parent = 0);

    CmdOutput       *cmdOutput;
    InputCmdLine       *inputCmdLine;


};

class NmConsole : public QMainWindow
{
    Q_OBJECT

public:
    NmConsole(QWidget *parent = 0);

signals:
    void savedLayoutToBeLoaded(QString);
    void saveLayout();

private slots:
    //void newFile();
    void open();
    //bool save();
    //bool saveAs();
    //void documentWasModified();
    void chopAndSend(const QString &text);
    void runScript();
    void runSelection();
    void setEasyNetHome();
    void setLazyNutBat();
protected:
    void closeEvent(QCloseEvent *event);

private:
    void createActions();
    void createMenus();
    void createToolBars();
//    bool maybeSave();
    void loadFile(const QString &fileName);
//    bool saveFile(const QString &fileName);

    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    void readSettings();
    void writeSettings();
    void runLazyNutBat();




    QString         lazyNutBat= "";
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


    NmCmd           *nmCmd;
    //NM              *lazyNut;
    LazyNut         *lazyNut;
    CommandSequencer    *commandSequencer;
    TreeModel       *objTaxonomyModel;
    LazyNutObjCatalogue  *objHash;
    QueryProcessor   *queryProcessor;
    ObjExplorer      *objExplorer;
    QDockWidget     *dockEdit;
    QDockWidget     *dockParse;
    QDockWidget     *dockExplorer;
    QDockWidget     *dockDesignWindow;
    QPlainTextEdit  *scriptEdit;
    DesignWindow    *designWindow;


    QMenu           *fileMenu;
    QMenu           *runMenu;
    QMenu           *settingsMenu;
    QToolBar        *fileToolBar;
    QToolBar        *runToolBar;
//    QAction         *newAct;
    QAction         *openAct;
//    QAction         *saveAct;
//    QAction         *saveAsAct;
    QAction         *exitAct;
    QAction         *runScriptAct;
    QAction         *runSelectionAct;
    QAction         *setEasyNetHomeAct;
    QAction         *setLazyNutBatAct;
    QAction         *queryModeAct;
};

#endif // NMCONSOLE_H
