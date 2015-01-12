
#include <QtWidgets>
#include <QtGlobal>
#include <QtDebug>
#include <QTreeView>
#include <QStringList>
#include <QEventLoop>
#include <QToolBar>
#include <QWebView>

#include <iostream>
#include <fstream>
#include <vector>

#include "driver.h"
//#include "parsenode.h"
#include "treeitem.h"
#include "querycontext.h"
#include "nmConsole.h"
#include "treemodel.h"
#include "lazynutobj.h"
#include "objexplorer.h"
#include "designwindow.h"
#include "codeeditor.h"
#include "lazynut.h"
#include "commandsequencer.h"
#include "sessionmanager.h"

InputCmdLine::InputCmdLine(QWidget *parent)
    : QLineEdit(parent)
{
    connect(this,SIGNAL(returnPressed()),
            this,SLOT(sendCommand()));
}

void InputCmdLine::sendCommand()
{
    QString line = text();
    emit commandReady(line);
    clear();
}

CmdOutput::CmdOutput(QWidget *parent)
    : QPlainTextEdit(parent)
{
    this->setStyleSheet("background-color : black; color : white;");
}

void CmdOutput::displayOutput(const QString & output)
{
    // http://stackoverflow.com/questions/13559990/qt-append-text-to-qplaintextedit-without-adding-newline-and-keep-scroll-at-the

    moveCursor (QTextCursor::End);
    insertPlainText (output);
    moveCursor (QTextCursor::End);
}





//NM::NM(QObject *parent)
//    : QProcess(parent)
//{
//}

//NM::~NM()
//{
//    terminate();
//}

//void NM::sendCommand(const QString & line)
//{
//    write(qPrintable(line + "\n"));
//}


//void NM::getNMError()
//{
//    QByteArray bytes = readAllStandardError();
//    emit outputReady(QString(bytes));
//}



//QueryProcessor::QueryProcessor(LazyNutObjCatalogue* objHash, TreeModel* objTaxonomyModel,  QWidget *parent):
//    QSplitter(parent), objHash(objHash), objTaxonomyModel(objTaxonomyModel),
//    contextQuerySize(0), lazyNutBuffer(""), treeOutput(""),
//    //rxEND(".*END:[^\\n]*\\n$")
//    rxEND("END:[^\\n]*\\n")
//{
//    context = new QueryContext;
//    driver = new lazyNutOutput::Driver(*context);

//    parseOutput = new CmdOutput(this);
//    parseOutput->setReadOnly(true);
//    addWidget(parseOutput);

//    connect(this,SIGNAL(treeReady(QString)),parseOutput,SLOT(displayOutput(QString)));
//}

//void QueryProcessor::testDesignWindow()
//{
//    // this member function should be removed and turned into a unit test.

//    emit beginObjHashModified();
//    objHash->insert("layerA",new LazyNutObj());
//    (*objHash)["layerA"]->appendProperty(QString{"name"},QVariant{"layerA"});
//    (*objHash)["layerA"]->appendProperty("type","layer");
//    (*objHash)["layerA"]->appendProperty("subtype","iac_layer");
//    (*objHash)["layerA"]->appendProperty("length", 24);
//    (*objHash)["layerA"]->appendProperty("incoming connections","connectionBA");

//    objHash->insert("layerB",new LazyNutObj());
//    (*objHash)["layerB"]->appendProperty("name","layerB");
//    (*objHash)["layerB"]->appendProperty("type","layer");
//    (*objHash)["layerB"]->appendProperty("subtype","iac_layer");
//    (*objHash)["layerB"]->appendProperty("length", 24);
//    (*objHash)["layerB"]->appendProperty("incoming connections",QStringList{"connectionAB","biasAB"});

//    objHash->insert("connectionAB",new LazyNutObj());
//    (*objHash)["connectionAB"]->appendProperty("name","connectionAB");
//    (*objHash)["connectionAB"]->appendProperty("type","connection");
//    (*objHash)["connectionAB"]->appendProperty("subtype","connection");
//    (*objHash)["connectionAB"]->appendProperty("length", 24);
//    (*objHash)["connectionAB"]->appendProperty("Source","layerA");
//    (*objHash)["connectionAB"]->appendProperty("Target","layerB");

//    emit endObjHashModified();

//    emit beginObjHashModified();
//    objHash->remove("connectionAB");
//    emit endObjHashModified();

//}


//void QueryProcessor::getTree(const QString &lazyNutOutput)
//{
//    lazyNutBuffer.append(lazyNutOutput);
//    int indexInLazyNutBuffer = rxEND.indexIn(lazyNutBuffer);
//    int lengthRemainder;
//    while (indexInLazyNutBuffer >=0)
//    {
//        lengthRemainder = lazyNutBuffer.size() - indexInLazyNutBuffer - rxEND.matchedLength();
//        QString remainder = lazyNutBuffer.right(lengthRemainder);
//        lazyNutBuffer.chop(lengthRemainder);
//        //qDebug() << lazyNutBuffer;
//        //qDebug() << lengthRemainder;
//        //qDebug() << remainder;
//        bool result = driver->parse_string(lazyNutBuffer.toStdString(), "lazyNutOutput");
//        if (result)
//        {
//            contextQuerySize = 0;
//            treeOutput = "";
//            if (context->root->childCount() > contextQuerySize)
//            {
//                for (int qi = contextQuerySize; qi < context->root->childCount(); ++qi)
//                {
//                    context->root->child(qi)->print(treeOutput);
//                }
//                //contextQuerySize = context->root->childCount();
//                //lazyNutBuffer = remainder; //"";
//           }
//           else
//           {
//               // treeOutput += ".";
//           }
//           //qDebug() << treeOutput;
//           emit treeReady(treeOutput);
//           processQueries();
//           emit resultAvailable(lazyNutBuffer);

//        }
//        else
//        {
//            //lazyNutBuffer.append(remainder);
//            //lazyNutBuffer = remainder;
//            qDebug() << "no result";
//        /*    if (context->begin_query)
//            {
//                context->root->removeLastChild();
//            }
//            context->begin_query = false;
//            lazyNutBuffer = ""; // to be deleted */
//        }
//        lazyNutBuffer = remainder;
//        indexInLazyNutBuffer = rxEND.indexIn(lazyNutBuffer);
//    }
//}

//void QueryProcessor::processQueries()
//{
//    QStringList recentlyModified;
//    bool objHashModified = false;
//    foreach (TreeItem* queryItem, context->root->children())
//    {
//        QString queryType = queryItem->data(0).toString();
//        if (queryType == "subtypes")
//        {
//            QString objectType = queryItem->data(1).toString();
//            QModelIndex objectIndex = objTaxonomyModel->index(0,0);
//            int row = 0;
//            while (objTaxonomyModel->data(objTaxonomyModel->index(row,0,objectIndex),Qt::DisplayRole).toString() != objectType &&
//                   row < objTaxonomyModel->rowCount(objectIndex))
//                ++row;
//            QModelIndex typeIndex = objTaxonomyModel->index(row,0,objectIndex);
//            if (typeIndex.isValid())
//            {
//                foreach (TreeItem* subtypeItem, queryItem->children())
//                {
//                    objTaxonomyModel->appendValue(subtypeItem->data(1).toString(),typeIndex);
//                }
//            }
//        }
//        else if (queryType == "recently_modified")
//        {
//            foreach (TreeItem* objectItem, queryItem->children())
//            {
//                recentlyModified.append(objectItem->data(1).toString());
//            }
//        }
//        else if (queryType == "description")
//        {
//            emit beginObjHashModified();
//            foreach (TreeItem* objectItem, queryItem->children())
//            {
//                QString objectName = objectItem->data(1).toString();
//                objHash->insert(objectName,new LazyNutObj());
//                foreach (TreeItem* propertyItem, objectItem->children())
//                {
//                    QString propertyKey = propertyItem->data(0).toString();
//                    QString propertyValue = propertyItem->data(1).toString();
//                    if (propertyValue.startsWith('[') && propertyValue.endsWith(']'))
//                        // todo: generate query list
//                        (*objHash)[objectName]->appendProperty(propertyKey,propertyValue);
//                    else if (propertyValue.contains(','))
//                        (*objHash)[objectName]->appendProperty(propertyKey,propertyValue.split(", "));
//                    else
//                        (*objHash)[objectName]->appendProperty(propertyKey,propertyValue);
//                }
//            }
//            objHashModified = true;
//        }
//    }
//    if (objHashModified)
//        emit endObjHashModified();
//    context->clearQueries();
//    foreach (QString objectName, recentlyModified)
//    {
//        QString query = "query 1 " + objectName + " description";
//        emit commandReady(query);
//    }
//    recentlyModified.clear();
//}


//NmCmd::NmCmd(QWidget *parent)
//    : QWidget(parent)
//{
//    QVBoxLayout *mainLayout = new QVBoxLayout(this);
//    cmdOutput = new CmdOutput(this);
//    cmdOutput->setReadOnly(true);
//    inputCmdLine = new InputCmdLine(this);
//    mainLayout->addWidget(cmdOutput);
//    mainLayout->addWidget(inputCmdLine);
//    setLayout(mainLayout);
//    setWindowTitle(tr("lazyNut cmd[*]"));
//}


NmConsole::NmConsole(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(QIcon(":/images/zebra.png"));
    QTextEdit* dummyEdit = new QTextEdit(this);
    dummyEdit->hide();
    setCentralWidget(dummyEdit);
//    createActions();

/*    welcomeScreen = new QTextEdit(this);
    QFile myfile(":/images/Welcome.html");
//    f = QFile("welcome.html");
    if (myfile.exists())
        myfile.open(QIODevice::ReadOnly);
    else
            int oops=1;
    QTextStream textStream(&myfile);
    QString line = textStream.readAll();
    myfile.close();
    welcomeScreen->insertHtml(line);
*/
    welcomeScreen = new QWebView(this);
    webWelcomeScreen = new QWebView(this);

    zebPic = new QLabel();
    QPixmap image(":/images/zebra.png");
    zebPic->setPixmap(image);

/*    dockZeb = new QDockWidget(this);
    dockZeb->setAllowedAreas(  Qt::RightDockWidgetArea);
    dockZeb->setWidget(zebPic);
    dockZeb->resize(200,200);
    addDockWidget(Qt::RightDockWidgetArea, dockZeb);
*/
    dockWelcome = new QDockWidget(tr("Welcome"), this);
    dockWelcome->setAllowedAreas(  Qt::TopDockWidgetArea );
    dockWelcome->setWidget(welcomeScreen);
    addDockWidget(Qt::TopDockWidgetArea, dockWelcome);

    dockWebWelcome = new QDockWidget(this);
    dockWebWelcome->setAllowedAreas(  Qt::BottomDockWidgetArea );
    dockWebWelcome->setWidget(webWelcomeScreen);
    addDockWidget(Qt::BottomDockWidgetArea, dockWebWelcome);

    lazyNutConsole = new QGroupBox("lazyNut console", this);
    cmdOutput = new CmdOutput(this);
    cmdOutput->setReadOnly(true);
    inputCmdLine = new InputCmdLine(this);
    QVBoxLayout *consoleLayout = new QVBoxLayout;
    consoleLayout->addWidget(cmdOutput);
    consoleLayout->addWidget(inputCmdLine);
    lazyNutConsole->setLayout(consoleLayout);


//   nmCmd = new NmCmd(this);
    dockParse = new QDockWidget(tr("lazyNut interpreter"), this);
//    dockParse->setAllowedAreas(  Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    dockParse->setWidget(lazyNutConsole);
    //addDockWidget(Qt::BottomDockWidgetArea, dockParse);
    addDockWidget(Qt::RightDockWidgetArea, dockParse);

//    scriptEdit = new CodeEditor(this);
    scriptEdit = new editWindow(this, newScriptAct, openAct);
    scriptEdit->textEdit->setReadOnly(false);
    highlighter = new Highlighter(scriptEdit->textEdit->document());

    dockEdit = new QDockWidget(tr("Untitled"), this);
    dockEdit->setAllowedAreas(  Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea);
    dockEdit->setWidget(scriptEdit);
    addDockWidget(Qt::LeftDockWidgetArea, dockEdit);


    CodeEditor *tmpEdit = new CodeEditor(this);
    tmpEdit->setReadOnly(true);
    highlighter = new Highlighter(tmpEdit->document());

    /*
//    QGraphicsSvgItem *svgViewer = new QGraphicsSvgItem("example.svg");
    QGraphicsScene *scene=new QGraphicsScene(QRect(10, 10, 680, 520));
    QGraphicsView *view=new QGraphicsView(this);
    QGraphicsPixmapItem *image1=new QGraphicsPixmapItem(QPixmap("zebra.png"));

    scene ->addItem(image1);
    view ->setScene(scene);
    view ->setGeometry(QRect(270, 35, 700, 540));
*/
    plotForm = new plotWindow();
    dockOutput = new QDockWidget(tr("Output"), this);
    dockOutput->setAllowedAreas(  Qt::LeftDockWidgetArea |
                                    Qt::RightDockWidgetArea);
    dockOutput->setWidget(plotForm);
    addDockWidget(Qt::LeftDockWidgetArea, dockOutput);

//    commandLog = new CodeEditor(this);
    commandLog = new editWindow(this, newLogAct, NULL, false, false); // no cut, no paste

    commandLog->textEdit->setReadOnly(true);
    highlighter2 = new Highlighter(commandLog->textEdit->document());

    dockCommandLog = new QDockWidget(tr("Command Log"), this);
    dockCommandLog->setAllowedAreas(  Qt::LeftDockWidgetArea |
                                        Qt::RightDockWidgetArea);
    dockCommandLog->setWidget(commandLog);
    addDockWidget(Qt::RightDockWidgetArea, dockCommandLog);


//    setCentralWidget(lazyNutConsole);
// //    nmCmd = new NmCmd(this);
// //    setCentralWidget(nmCmd);
//    scriptEditor = new LazyNutScriptEditor(this);
//    dockEdit = new QDockWidget(tr("my_nm_script"), this);
//    dockEdit->setAllowedAreas(  Qt::LeftDockWidgetArea |
//                                Qt::RightDockWidgetArea);
//    dockEdit->setWidget(scriptEditor);
//    addDockWidget(Qt::RightDockWidgetArea, dockEdit);
//    dockEdit->hide();

    objTaxonomyModel = new TreeModel(QStringList{"Object taxonomy"},this);
    objTaxonomyModel->appendValue(QString{"object"});
    // type
    QModelIndex parentIndex = objTaxonomyModel->index(0,0);
    foreach (QString type, lazyNutObjTypes)
        objTaxonomyModel->appendValue(type,parentIndex);

    objCatalogue = new LazyNutObjCatalogue();

    //queryProcessor = new QueryProcessor(objCatalogue,objTaxonomyModel,this);
    objExplorer = new ObjExplorer(objCatalogue,objTaxonomyModel,this);

    dockExplorer = new QDockWidget(tr("Object Explorer"), this);
    dockExplorer->setAllowedAreas( Qt::RightDockWidgetArea );
    dockExplorer->setWidget(objExplorer);
    addDockWidget(Qt::RightDockWidgetArea,dockExplorer);



    designWindow = new DesignWindow(this);
    designWindow->setObjCatalogue(objCatalogue);
    connect(designWindow,SIGNAL(showObj(LazyNutObj*,LazyNutObjCatalogue*)),
            objExplorer,SLOT(setObj(LazyNutObj*,LazyNutObjCatalogue*)));
    connect(objExplorer,SIGNAL(objSelected(QString)),
            designWindow,SIGNAL(objSelected(QString)));

    dockDesignWindow = new QDockWidget(tr("Design Window"), this);
    dockDesignWindow->setAllowedAreas(  Qt::LeftDockWidgetArea |
                                    Qt::RightDockWidgetArea);
    dockDesignWindow->setWidget(designWindow);
    addDockWidget(Qt::LeftDockWidgetArea, dockDesignWindow);
        //dockDesignWindow->hide();

    showWelcomeView();

    // signals begin/endObjHashModified are defined in SessionManager and ObjExplorer.
    // When a description query is parsed by the SessionManager, it sends those signals
    // to the ObjExplorer, which in turn trigger slots sendBegin/EndResetModel in
    // LazyNutObjTableModel that update its internal representation.
    // All this signal/slot chain is necessary in order to call
    // the protected functions QAbstractItemModel::begin/endResetModel.
    // Those functions need to be called because SessionManager modifies
    // the LazyNutObjCatalogue object directly, which is the underlying data structure
    // to LazyNutObjTableModel, i.e. it does not make use of the QAbstractTableModel
    // API implemented by LazyNutObjTableModel.




//    createActions();
//    createMenus();
//    createToolBars();

    readSettings();

    //queryProcessor->testDesignWindow();

    setCurrentFile(scriptEdit,"Untitled");




    //lazyNut = new NM(this);
    lazyNut = new LazyNut(this);



    connect(lazyNut,SIGNAL(outputReady(QString)),
            cmdOutput,SLOT(displayOutput(QString)));
//    connect(lazyNut,SIGNAL(outputReady(QString)),
//            queryProcessor,SLOT(getTree(QString)));

    connect(this,SIGNAL(savedLayoutToBeLoaded(QString)),
            designWindow,SIGNAL(savedLayoutToBeLoaded(QString)));
    connect(this,SIGNAL(saveLayout()),
            designWindow,SIGNAL(saveLayout()));

    //easyNetHome = QString(qgetenv("EASYNET_HOME"));
    //lazyNutBat = ""; // debug
    //easyNetHome = ""; // debug
    if (lazyNutBat.isEmpty())
    {
        if (easyNetHome.isEmpty())
        {
            QMessageBox::warning(this, "warning",QString("Please select a valid %1 file from the menu Settings -> Set %1\n"
                                 "or a valid easyNet home directory using the menu Settings -> Set easyNet home directory").arg(lazyNutBasename));
        }
        else
        {
            lazyNutBat = easyNetHome + QString("/%1/nm_files/%2").arg(binDir).arg(lazyNutBasename);
        }
    }
    if (!lazyNutBat.isEmpty())
        runLazyNutBat();

    //    commandSequencer = new CommandSequencer(lazyNut,this);
    //    connect(queryProcessor,SIGNAL(resultAvailable(QString)),
    //            commandSequencer,SLOT(receiveResult(QString)));
        connect(inputCmdLine,SIGNAL(commandReady(QString)),
                this,SLOT(runCmd(QString)));
    //    connect(queryProcessor,SIGNAL(commandReady(QString)),
    //            commandSequencer,SLOT(runCommand(QString)));

    sessionManager = new SessionManager(lazyNut,objCatalogue,objTaxonomyModel,this);
    connect(lazyNut,SIGNAL(outputReady(QString)),sessionManager,SLOT(parseLazyNutOutput(QString)));
    connect(sessionManager,SIGNAL(beginObjHashModified()),objExplorer,SIGNAL(beginObjHashModified()));
    connect(sessionManager,SIGNAL(endObjHashModified()),objExplorer,SIGNAL(endObjHashModified()));
    connect(sessionManager,SIGNAL(endObjHashModified()),
            designWindow,SLOT(objCatalogueChanged()));

    createActions();
    createMenus();
    createToolBars();

}

void NmConsole::readSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    easyNetHome = settings.value("easyNetHome","").toString();
    lazyNutBat = settings.value("lazyNutBat","").toString();
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    scriptsDir = settings.value("scriptsDir", easyNetHome + "/Models").toString();
    resize(size);
    move(pos);
}

void NmConsole::writeSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    settings.setValue("easyNetHome", easyNetHome);
    settings.setValue("lazyNutBat",lazyNutBat);
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("scriptsDir",scriptsDir);
    //  settings.setValue("nmExe",nmExe);
}



void NmConsole::closeEvent(QCloseEvent *event)
{
        writeSettings();
        lazyNut->kill();
        // wait until the scene has saved its layout
//        QEventLoop waitUntilLayoutSaved;
//        connect(designWindow,SIGNAL(layoutSaveAttempted()),
//                &waitUntilLayoutSaved,SLOT(quit()));
        emit saveLayout();
//        waitUntilLayoutSaved.exec();
        event->accept();
}

void NmConsole::newScriptFile()
{
    newFile(scriptEdit);
}

void NmConsole::newLogFile()
{
    newFile(commandLog);
}

void NmConsole::newFile(editWindow* window)
{
    if (!(sender()))
            return;
    if (window->maybeSave())
    {
        window->textEdit->clear();
        if (window==scriptEdit)
            setCurrentFile(window,"");
    }
}

void NmConsole::open()
{
 //   if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,tr("Open script"), scriptsDir, tr("Script Files (*.eNs *.eNm)"));
        if (!fileName.isEmpty())
        {
            loadFile(fileName);
            dockEdit->show();
            showCodeView();
        }
 //   }
}

/*bool NmConsole::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool NmConsole::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.exec();
    QStringList files = dialog.selectedFiles();

    if (files.isEmpty())
        return false;

    return saveFile(files.at(0));
}*/

void NmConsole::runSelection()
{

    sessionManager->runSelection(scriptEdit->textEdit->getSelectedText());
}

void NmConsole::runCmd(QString cmd)
{
    sessionManager->runSelection(QStringList(cmd));
}

void NmConsole::runModel()
{
    sessionManager->runModel(scriptEdit->textEdit->getAllText());
    //emit savedLayoutToBeLoaded(curJson);
}

void NmConsole::echoCommand(const QString &line)
{
//    QString return_line = runCmd(line);
    runCmd(line);
//    if (return_line.size() > 1)
//    commandLog->textEdit->insertPlainText(return_line);
        commandLog->textEdit->insertPlainText(line);
}

void NmConsole::setEasyNetHome()
{
    easyNetHome = QFileDialog::getExistingDirectory(this,tr("Please select your easyNet home directory.\n"));
    lazyNutBat = easyNetHome + QString("/%1/nm_files/%2").arg(binDir).arg(lazyNutBasename);
    runLazyNutBat();
}

void NmConsole::setLazyNutBat()
{
    lazyNutBat = QFileDialog::getOpenFileName(this,QString(tr("Please select your %1 file.")).arg(lazyNutBasename),
                                              easyNetHome,QString("*.%1").arg(lazyNutExt));
    runLazyNutBat();
}

void NmConsole::showPauseState(bool isPaused)
{
    if (isPaused)
        pauseAct->setIconText("RESUME");
    else
        pauseAct->setIconText("PAUSE");
}


void NmConsole::runLazyNutBat()
{
    lazyNut->setWorkingDirectory(QFileInfo(lazyNutBat).absolutePath());
    lazyNut->start(lazyNutBat);
    if (lazyNut->state() == QProcess::NotRunning)
    {
        QMessageBox::critical(this, "critical",
        QString("%1 script not running or not found.\n"
                "Please select a valid %1 file from the menu Settings -> Set %1\n"
                "or a valid easyNet home directory using the menu Settings -> Set easyNet home directory").arg(lazyNutBasename));
    }
}

//void NmConsole::chopAndSend(const QString & text)
//{
//    QStringList commandList = text.split("\u2029");
//    commandSequencer->runCommands(commandList);
//    QStringList lines = text.split("\u2029");
//    foreach (QString line, lines)
//    {
//        lazyNut->sendCommand(line + "\n");
//    }
//}


/*void NmConsole::documentWasModified()
{
    setWindowModified(scriptEdit->document()->isModified());
}*/


void NmConsole::createActions()
{
    welcomeAction = new QAction(QIcon(":/images/zebra_64x64.png"), tr("&Welcome"),
                               this);
//    welcomeAction->setShortcuts(QKeySequence::New);
    welcomeAction->setStatusTip(tr("Welcome to easyNet"));
    connect(welcomeAction, SIGNAL(triggered()), this, SLOT(showWelcomeView()));

    viewModelAction = new QAction(QIcon(":/images/layers-8x.png"), tr("&Model"),
                               this);
//    viewModelAction->setShortcuts(QKeySequence::New);
    viewModelAction->setStatusTip(tr("Display model view"));
    connect(viewModelAction, SIGNAL(triggered()), this, SLOT(showModelView()));

    viewTrialAction = new QAction(QIcon(":/images/cog-8x.png"), tr("&Trial"),
                               this);
//    viewTrialAction->setShortcuts(QKeySequence::New);
    viewTrialAction->setStatusTip(tr("Display trial editor"));
    connect(viewTrialAction, SIGNAL(triggered()), this, SLOT(showTrialView()));

    viewInputAction = new QAction(QIcon(":/images/list-8x.png"), tr("&Input"),
                               this);
//    viewInputAction->setShortcuts(QKeySequence::New);
    viewInputAction->setStatusTip(tr("Display input view"));
    connect(viewInputAction, SIGNAL(triggered()), this, SLOT(showInputView()));

    viewOutputAction = new QAction(QIcon(":/images/bar-chart-8x.png"), tr("&Output"),
                               this);
//    viewOutputAction->setShortcuts(QKeySequence::New);
    viewOutputAction->setStatusTip(tr("Display output view"));
    connect(viewOutputAction, SIGNAL(triggered()), this, SLOT(showOutputView()));

    viewParamsAction = new QAction(QIcon(":/images/dial-8x.png"), tr("&Parameters"),
                               this);
//    viewParamsAction->setShortcuts(QKeySequence::New);
    viewParamsAction->setStatusTip(tr("Display parameter view"));
    connect(viewParamsAction, SIGNAL(triggered()), this, SLOT(showParameterView()));

    viewInterpreterAction = new QAction(QIcon(":/images/terminal-8x.png"), tr("&Interpreter"),
                               this);
//    viewInterpreterAction->setShortcuts(QKeySequence::New);
    viewInterpreterAction->setStatusTip(tr("Display interpreter view"));
    connect(viewInterpreterAction, SIGNAL(triggered()), this, SLOT(showInterpreterView()));

    viewCodeAction = new QAction(QIcon(":/images/code-8x.png"), tr("&Code"),
                               this);
//    viewCodeAction->setShortcuts(QKeySequence::New);
    viewCodeAction->setStatusTip(tr("Display code view"));
    connect(viewCodeAction, SIGNAL(triggered()), this, SLOT(showCodeView()));

    runAction = new QAction(QIcon(":/images/media-play-8x.png"),tr("&Run"), this);
    runAction->setStatusTip(tr("Run"));
    connect(runAction,SIGNAL(triggered()),this, SLOT(run()));

    newScriptAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newScriptAct->setShortcuts(QKeySequence::New);
//    newScriptAct->setStatusTip(tr("Create a new file"));
    connect(newScriptAct, SIGNAL(triggered()), this, SLOT(newScriptFile()));

    newLogAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newLogAct->setShortcuts(QKeySequence::New);
//    newLogAct->setStatusTip(tr("Create a new file"));
    connect(newLogAct, SIGNAL(triggered()), this, SLOT(newLogFile()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));



    //saveAct = new QAction(tr("&Save"), this);
    //saveAct->setShortcuts(QKeySequence::Save);
    //saveAct->setStatusTip(tr("Save the document to disk"));
    //connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    //saveAsAct = new QAction(tr("Save &As..."), this);
    //saveAsAct->setShortcuts(QKeySequence::SaveAs);
    //saveAsAct->setStatusTip(tr("Save the document under a new name"));
    //connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    runSelectionAct = new QAction(tr("Run se&lection"), this);
    runSelectionAct->setStatusTip(tr("Run selected text"));
    connect(runSelectionAct,SIGNAL(triggered()),this, SLOT(runSelection()));

    runScriptAct = new QAction(tr("&Run script"), this);
    runScriptAct->setStatusTip(tr("Run script"));
    connect(runScriptAct,SIGNAL(triggered()),this, SLOT(runModel()));

    setEasyNetHomeAct = new QAction(tr("Set easyNet home directory"), this);
    setEasyNetHomeAct->setStatusTip(tr("Set easyNet home directory"));
    connect(setEasyNetHomeAct,SIGNAL(triggered()),this, SLOT(setEasyNetHome()));

    setLazyNutBatAct = new QAction(QString(tr("Set %1").arg(lazyNutBasename)), this);
    setLazyNutBatAct->setStatusTip(QString(tr("Set %1").arg(lazyNutBasename)));
    connect(setLazyNutBatAct,SIGNAL(triggered()),this, SLOT(setLazyNutBat()));

    synchModeAct = new QAction("run in synch mode",this);
    synchModeAct->setCheckable(true);
    synchModeAct->setChecked(false);
    connect(synchModeAct,SIGNAL(toggled(bool)),sessionManager,SLOT(setSynchMode(bool)));

    stopAct = new QAction("STOP",this);
    connect(stopAct,SIGNAL(triggered()),sessionManager,SLOT(stop()));
    pauseAct = new QAction("PAUSE",this);
    connect(pauseAct,SIGNAL(triggered()),sessionManager,SLOT(pause()));
    connect(sessionManager,SIGNAL(isPaused(bool)),this,SLOT(showPauseState(bool)));
}

void NmConsole::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    //fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    //fileMenu->addAction(saveAct);
    //fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    runMenu = menuBar()->addMenu(tr("&Run"));
    runMenu->addAction(runSelectionAct);
    runMenu->addAction(runScriptAct);
    runMenu->addAction(pauseAct);
    runMenu->addAction(stopAct);

    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    settingsMenu->addAction(setEasyNetHomeAct);
    settingsMenu->addAction(setLazyNutBatAct);
    settingsMenu->addAction(synchModeAct);
}

void NmConsole::createToolBars()
{
    QLabel *spacing = new QLabel(tr("____________"));
/*    spacing->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    spacing->setContentsMargins(0,0,0,0);
    spacing->setSizePolicy(QSizePolicy::MinimumExpanding,
                           QSizePolicy::MinimumExpanding);
*/

/*    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(openAct);
    runToolBar = addToolBar(tr("Run"));
    runToolBar->addAction(runSelectionAct);
    runToolBar->addAction(runScriptAct);
*/

//    runToolBar->addAction(pauseAct);
//    runToolBar->addAction(stopAct);

    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;

    std::vector <QToolButton*> buttons;
//    buttons = new std::vector <QToolButton>;
//    QToolButton *button2;
    int numButtons=9;
    for (int i=0;i<numButtons;i++)
    {
        buttons.push_back(new QToolButton(this));
        buttons[i]->setAutoRaise(true);
        buttons[i]->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }

    buttons[0]->addAction(welcomeAction);
    buttons[0]->setDefaultAction(welcomeAction);
    buttons[1]->addAction(viewModelAction);
    buttons[1]->setDefaultAction(viewModelAction);
    buttons[2]->addAction(viewParamsAction);
    buttons[2]->setDefaultAction(viewParamsAction);
    buttons[3]->addAction(viewTrialAction);
    buttons[3]->setDefaultAction(viewTrialAction);
    buttons[4]->addAction(viewInputAction);
    buttons[4]->setDefaultAction(viewInputAction);
    buttons[5]->addAction(viewOutputAction);
    buttons[5]->setDefaultAction(viewOutputAction);
    buttons[6]->addAction(viewInterpreterAction);
    buttons[6]->setDefaultAction(viewInterpreterAction);
    buttons[7]->addAction(viewCodeAction);
    buttons[7]->setDefaultAction(viewCodeAction);
    buttons[8]->addAction(runAction);
    buttons[8]->setDefaultAction(runAction);

    infoToolBar = new QToolBar(this);
    infoToolBar->setStyleSheet("QToolButton::menu-indicator {image: url(myindicator.png); } \
//                subcontrol-position: right center; subcontrol-origin: padding; left: -2px;}"
    "QToolButton {font-size: 9pt; color: \"white\"; icon-size: 30px; min-width: 5em; padding: 3px;} "
    "QToolButton:pressed {border: 2px solid #8f8f91; border-radius: 6px; background-color:red;}"
    "QLabel { font-size: 8pt; color: \"white\"; icon-size: 30px; } "
    "QToolBar { background: qlineargradient(x1: 0, y1: 0,    x2: 0, y2: 1, "
    "stop: 0 #66e, stop: 1 #bbf); background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1, "
    "stop: 0 #bbf, stop: 1 #55f) } ");

/*    QComboBox* parametersCB = new QComboBox;
    QLabel* modelLabel = new QLabel("IA");
    QLabel* trialLabel = new QLabel("masked_priming_ldt");
    QLabel* inputLabel = new QLabel("Davis_Lupker_06");
*/
    for (int i=0;i<numButtons;i++)
    {
        vbox->addWidget(buttons[i]);
        if (i==7)
            vbox->addWidget(spacing);
    }
//    vbox->addWidget(spacing);
//    vbox->addWidget(modelLabel);
//    vbox->addWidget(trialLabel);
//    vbox->addWidget(inputLabel);

    w->setLayout(vbox);
    infoToolBar->addWidget(w);
    addToolBar(Qt::LeftToolBarArea, infoToolBar);
}


void NmConsole::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    scriptEdit->textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(scriptEdit,fileName); // assuming that this is called by scriptEdit
    scriptsDir = QFileInfo(fileName).absolutePath();

    //statusBar()->showMessage(tr("File loaded"), 2000);
}

/*bool NmConsole::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("nm Console"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << scriptEdit->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    //statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}*/

void NmConsole::setCurrentFile(editWindow *window, const QString &fileName)
{
    window->setCurrentFile(fileName);
    window->textEdit->document()->setModified(false);
    setWindowModified(false);

    curJson = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName().append(".json"));
}

QString NmConsole::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

/*bool NmConsole::maybeSave()
{
    if (scriptEdit->document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("nm console"),
                     tr("The script has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}*/

void NmConsole::hideAllDocks()
{
//    dockZeb->hide();
    dockWelcome->hide();
    dockWebWelcome->hide();
    dockEdit->hide();
    dockParse->hide();
//    dockInput->hide();
    dockOutput->hide();
    dockExplorer->hide();
    dockDesignWindow->hide();
    dockCommandLog->hide();
}

void NmConsole::showWelcomeView()
{
    hideAllDocks();
    setCorner( Qt::TopLeftCorner, Qt::TopDockWidgetArea );
    setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
    setCorner( Qt::BottomLeftCorner, Qt::BottomDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::BottomDockWidgetArea );

//    dockZeb->show();
    welcomeScreen->setUrl(QUrl("qrc:///images/Welcome.html"));
    webWelcomeScreen->setUrl(tr("http://www.adelmanlab.org/easyNet/"));
    dockWebWelcome->show();
    dockWelcome->show();
}


void NmConsole::showCodeView()
{
    hideAllDocks();
    dockEdit->show();
    dockCommandLog->show();
    dockParse->show();
}

void NmConsole::showModelView()
{
    hideAllDocks();
    dockDesignWindow->show();
    dockExplorer->show();
//    dockDesignWindow->resize(500, dockDesignWindow->height());
}

void NmConsole::showTrialView()
{
    hideAllDocks();
//    dockEdit->show();
//    dockEdit->resize(500, dockEdit->height());
}

void NmConsole::showInputView()
{
    hideAllDocks();
//    dockEdit->show();
//    dockEdit->resize(500, dockEdit->height());
}


void NmConsole::showOutputView()
{
    hideAllDocks();
    dockOutput->show();
}

void NmConsole::showParameterView()
{
    hideAllDocks();
//    dockEdit->show();
//    dockEdit->resize(500, dockEdit->height());
}

void NmConsole::showInterpreterView()
{
    hideAllDocks();
    dockParse->show();
}

//QDockWidget     *dockExplorer;


void NmConsole::run()
{
    runModel(); // ultimately this will have different action depending on which mode is active

}



LazyNutScriptEditor::LazyNutScriptEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(true);
}

QStringList LazyNutScriptEditor::getSelectedText()
{
    return textCursor().selectedText().split("\u2029");
}

QStringList LazyNutScriptEditor::getAllText()
{
    return toPlainText().split("\n");
}


