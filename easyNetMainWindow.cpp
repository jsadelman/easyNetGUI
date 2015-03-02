
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

#include "easyNetMainWindow.h"
#include "treemodel.h"
#include "objexplorer.h"
#include "designwindow.h"
#include "codeeditor.h"
#include "commandsequencer.h"
#include "sessionmanager.h"
#include "highlighter.h"
#include "codeeditor.h"
#include "editwindow.h"
#include "plotwindow.h"

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
    setStyleSheet("background-color : black; color : white;");
}

void CmdOutput::displayOutput(const QString & output)
{
    // http://stackoverflow.com/questions/13559990/qt-append-text-to-qplaintextedit-without-adding-newline-and-keep-scroll-at-the

    moveCursor (QTextCursor::End);
    insertPlainText (output);
    moveCursor (QTextCursor::End);
}



EasyNetMainWindow::EasyNetMainWindow(QWidget *parent)
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

    lazyNutInterpreter = new QGroupBox(this);
    cmdOutput = new CmdOutput(this);
    cmdOutput->setReadOnly(true);
    inputCmdLine = new InputCmdLine(this);
    QVBoxLayout *interpreterLayout = new QVBoxLayout;
    interpreterLayout->addWidget(cmdOutput);
    interpreterLayout->addWidget(inputCmdLine);
    lazyNutInterpreter->setLayout(interpreterLayout);


//   nmCmd = new NmCmd(this);
    dockInterpreter = new QDockWidget(tr("lazyNut interpreter"), this);
//    dockParse->setAllowedAreas(  Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    dockInterpreter->setWidget(lazyNutInterpreter);
    //addDockWidget(Qt::BottomDockWidgetArea, dockParse);
    addDockWidget(Qt::RightDockWidgetArea, dockInterpreter);

//    scriptEdit = new CodeEditor(this);
    scriptEdit = new EditWindow(this, newScriptAct, openAct, false);
    //scriptEdit->textEdit->setReadOnly(false);
    highlighter = new Highlighter(scriptEdit->textEdit->document());

    dockEdit = new QDockWidget(tr("Untitled"), this);
    dockEdit->setAllowedAreas(  Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea);
    dockEdit->setWidget(scriptEdit);
    addDockWidget(Qt::LeftDockWidgetArea, dockEdit);


//    CodeEditor *tmpEdit = new CodeEditor(this);
//    tmpEdit->setReadOnly(true);
//    highlighter = new Highlighter(tmpEdit->document());

    /*
//    QGraphicsSvgItem *svgViewer = new QGraphicsSvgItem("example.svg");
    QGraphicsScene *scene=new QGraphicsScene(QRect(10, 10, 680, 520));
    QGraphicsView *view=new QGraphicsView(this);
    QGraphicsPixmapItem *image1=new QGraphicsPixmapItem(QPixmap("zebra.png"));

    scene ->addItem(image1);
    view ->setScene(scene);
    view ->setGeometry(QRect(270, 35, 700, 540));
*/
    plotForm = new PlotWindow(this);
    dockOutput = new QDockWidget(tr("Output"), this);
    dockOutput->setAllowedAreas(  Qt::LeftDockWidgetArea |
                                    Qt::RightDockWidgetArea);
    dockOutput->setWidget(plotForm);
    addDockWidget(Qt::LeftDockWidgetArea, dockOutput);

//    commandLog = new CodeEditor(this);
    commandLog = new EditWindow(this, newLogAct, NULL, true); // no cut, no paste

    //commandLog->textEdit->setReadOnly(true);
    highlighter2 = new Highlighter(commandLog->textEdit->document());

    dockCommandLog = new QDockWidget(tr("Command Log"), this);
    dockCommandLog->setAllowedAreas(  Qt::LeftDockWidgetArea |
                                        Qt::RightDockWidgetArea);
    dockCommandLog->setWidget(commandLog);
    addDockWidget(Qt::RightDockWidgetArea, dockCommandLog);



//    objTaxonomyModel = new TreeModel(QStringList{"Object taxonomy"},this);
//    objTaxonomyModel->appendValue(QString{"object"});
//    // type
//    QModelIndex parentIndex = objTaxonomyModel->index(0,0);
//    foreach (QString type, lazyNutObjTypes)
//        objTaxonomyModel->appendValue(type,parentIndex);

    objectCatalogue = new LazyNutObjectCatalogue;

    objExplorer = new ObjExplorer(objectCatalogue,this);

    dockExplorer = new QDockWidget(tr("Object Explorer"), this);
    dockExplorer->setAllowedAreas( Qt::RightDockWidgetArea );
    dockExplorer->setWidget(objExplorer);
    addDockWidget(Qt::RightDockWidgetArea,dockExplorer);



    designWindow = new DesignWindow(objectCatalogue, this);
    connect(designWindow,SIGNAL(objectSelected(QString)),
            objExplorer,SLOT(setObjFromObjName(QString)));
    connect(objExplorer,SIGNAL(objectSelected(QString)),
            designWindow,SLOT(dispatchObjectSelected(QString)));


    dockDesignWindow = new QDockWidget(tr("Design Window"), this);
    dockDesignWindow->setAllowedAreas(  Qt::LeftDockWidgetArea |
                                    Qt::RightDockWidgetArea);
    dockDesignWindow->setWidget(designWindow);
    addDockWidget(Qt::LeftDockWidgetArea, dockDesignWindow);




    readSettings();
    setCurrentFile(scriptEdit,"Untitled");

    sessionManager = new SessionManager(this);
    connect(sessionManager,SIGNAL(descriptionReady(QDomDocument*)),
            objExplorer,SLOT(updateLazyNutObjCatalogue(QDomDocument*)));
    connect(sessionManager,SIGNAL(updateDiagramScene()),
            designWindow,SLOT(updateDiagramScene()));
    connect(sessionManager,SIGNAL(userLazyNutOutputReady(QString)),cmdOutput,SLOT(displayOutput(QString)));
    connect(sessionManager,SIGNAL(lazyNutNotRunning()),this,SLOT(lazyNutNotRunning()));
    connect(this,SIGNAL(savedLayoutToBeLoaded(QString)),designWindow,SIGNAL(savedLayoutToBeLoaded(QString)));
    connect(this,SIGNAL(saveLayout()),designWindow,SIGNAL(saveLayout()));


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
        sessionManager->startLazyNut(lazyNutBat);
        connect(inputCmdLine,SIGNAL(commandReady(QString)),
                this,SLOT(runCmd(QString)));

    createActions();
    createMenus();
    createToolBars();
    showViewMode(Welcome);
}


void EasyNetMainWindow::readSettings()
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

void EasyNetMainWindow::writeSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    settings.setValue("easyNetHome", easyNetHome);
    settings.setValue("lazyNutBat",lazyNutBat);
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("scriptsDir",scriptsDir);
}



void EasyNetMainWindow::closeEvent(QCloseEvent *event)
{
        writeSettings();
        sessionManager->killLazyNut();
        emit saveLayout();
        event->accept();
}

void EasyNetMainWindow::initViewActions()
{
    viewActionIcons.insert(Welcome, new QIcon(":/images/zebra_64x64.png"));
    viewActionIcons.insert(Model, new QIcon(":/images/layers-8x.png"));
    viewActionIcons.insert(Trial, new QIcon(":/images/cog-8x.png"));
    viewActionIcons.insert(Input, new QIcon(":/images/list-8x.png"));
    viewActionIcons.insert(Output, new QIcon(":/images/bar-chart-8x.png"));
    viewActionIcons.insert(Params, new QIcon(":/images/dial-8x.png"));
    viewActionIcons.insert(Interpreter, new QIcon(":/images/terminal-8x.png"));
    viewActionIcons.insert(Code, new QIcon(":/images/code-8x.png"));

    viewActionTexts.insert(Welcome, tr("&Welcome"));
    viewActionTexts.insert(Model, tr("&Model"));
    viewActionTexts.insert(Trial, tr("&Trial"));
    viewActionTexts.insert(Input, tr("&Input"));
    viewActionTexts.insert(Output, tr("&Output"));
    viewActionTexts.insert(Params, tr("&Parameters"));
    viewActionTexts.insert(Interpreter, tr("&Interpreter"));
    viewActionTexts.insert(Code, tr("&Code"));

    viewActionStatusTips.insert(Welcome, tr("Welcome to easyNet"));
    viewActionStatusTips.insert(Model, tr("Display model view"));
    viewActionStatusTips.insert(Trial, tr("Display trial editor"));
    viewActionStatusTips.insert(Input, tr("Display input view"));
    viewActionStatusTips.insert(Output, tr("Display output view"));
    viewActionStatusTips.insert(Params, tr("Display parameter view"));
    viewActionStatusTips.insert(Interpreter, tr("Display interpreter view"));
    viewActionStatusTips.insert(Code, tr("Display code view"));
}

void EasyNetMainWindow::createViewActions()
{
    initViewActions();
    viewModeSignalMapper = new QSignalMapper(this);
    // iterate over ViewMode enum
    // http://stackoverflow.com/questions/261963/how-can-i-iterate-over-an-enum
    // http://stackoverflow.com/questions/1390703/enumerate-over-an-enum-in-c
    for (int viewModeInt = ViewMode_BEGIN; viewModeInt != ViewMode_END; viewModeInt++)
    {
        // ViewMode viewMode = static_cast<ViewMode>(viewModeInt);
        QAction *action = new QAction(this);
        viewActions.insert(viewModeInt, action);
        action->setIcon(*viewActionIcons.at(viewModeInt));
        action->setText(viewActionTexts.at(viewModeInt));
        action->setStatusTip(viewActionStatusTips.at(viewModeInt));
        connect(action, SIGNAL(triggered()), viewModeSignalMapper, SLOT(map()));
        viewModeSignalMapper->setMapping(action, viewModeInt);
    }
    connect(viewModeSignalMapper,SIGNAL(mapped(int)),this,SIGNAL(viewModeClicked(int)));
    connect(this,SIGNAL(viewModeClicked(int)),this,SLOT(showViewMode(int)));
}

void EasyNetMainWindow::newScriptFile()
{
    newFile(scriptEdit);
}

void EasyNetMainWindow::newLogFile()
{
    newFile(commandLog);
}

void EasyNetMainWindow::newFile(EditWindow* window)
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

void EasyNetMainWindow::open()
{
 //   if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,tr("Open script"), scriptsDir, tr("Script Files (*.eNs *.eNm)"));
        if (!fileName.isEmpty())
        {
            loadFile(fileName);
//            dockEdit->show();
//            showCodeView();
            showViewMode(Code);
        }
 //   }
}


void EasyNetMainWindow::runSelection()
{
    sessionManager->runSelection(scriptEdit->textEdit->getSelectedText());
}

void EasyNetMainWindow::runCmd(QString cmd)
{
    sessionManager->runSelection(QStringList(cmd));
}

void EasyNetMainWindow::runModel()
{
    sessionManager->runModel(scriptEdit->textEdit->getAllText());
    //emit savedLayoutToBeLoaded(curJson);
}

void EasyNetMainWindow::echoCommand(const QString &line)
{
//    QString return_line = runCmd(line);
    runCmd(line);
//    if (return_line.size() > 1)
//    commandLog->textEdit->insertPlainText(return_line);
        commandLog->textEdit->insertPlainText(line);
}

void EasyNetMainWindow::setEasyNetHome()
{
    easyNetHome = QFileDialog::getExistingDirectory(this,tr("Please select your easyNet home directory.\n"));
    lazyNutBat = easyNetHome + QString("/%1/nm_files/%2").arg(binDir).arg(lazyNutBasename);
    sessionManager->startLazyNut(lazyNutBat);
}

void EasyNetMainWindow::setLazyNutBat()
{
    lazyNutBat = QFileDialog::getOpenFileName(this,QString(tr("Please select your %1 file.")).arg(lazyNutBasename),
                                              easyNetHome,QString("*.%1").arg(lazyNutExt));
    sessionManager->startLazyNut(lazyNutBat);
}

void EasyNetMainWindow::showPauseState(bool isPaused)
{
    if (isPaused)
        pauseAct->setIconText("RESUME");
    else
        pauseAct->setIconText("PAUSE");
}

void EasyNetMainWindow::lazyNutNotRunning()
{
    QMessageBox::critical(this, "critical",
    QString("%1 script not running or not found.\n"
            "Please select a valid %1 file from the menu Settings -> Set %1\n"
            "or a valid easyNet home directory using the menu Settings -> Set easyNet home directory").arg(lazyNutBasename));
}

void EasyNetMainWindow::displayVersion(QString version)
{
    QMessageBox::about(this, "Version",QString("LazyNut version is:\n%1").arg(version));
}


void EasyNetMainWindow::createActions()
{
    createViewActions();

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

    stopAct = new QAction("STOP",this);
    connect(stopAct,SIGNAL(triggered()),sessionManager,SLOT(stop()));
    pauseAct = new QAction("PAUSE",this);
    connect(pauseAct,SIGNAL(triggered()),sessionManager,SLOT(pause()));
    connect(sessionManager,SIGNAL(isPaused(bool)),this,SLOT(showPauseState(bool)));

    versionAct = new QAction("Version",this);
    connect(versionAct,SIGNAL(triggered()),this,SLOT(requestVersion()));
}

void EasyNetMainWindow::requestVersion()
{
    sessionManager->commandSequencer->runCommand("version", JobOrigin::GUI, this, false, SLOT(displayVersion(QString)));
}

void EasyNetMainWindow::createMenus()
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
//    settingsMenu->addAction(synchModeAct);

    aboutMenu = menuBar()->addMenu(tr("&About"));
    aboutMenu->addAction(versionAct);
}

void EasyNetMainWindow::createToolBars()
{

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



    infoToolBar = new QToolBar(this);
    infoToolBar->setStyleSheet("QToolButton::menu-indicator {image: url(myindicator.png); \
                subcontrol-position: right center; subcontrol-origin: padding; left: -2px;}"
    "QToolButton {font-size: 9pt; color: \"white\"; icon-size: 30px; min-width: 5em; padding: 3px;} "
    "QToolButton:pressed {border: 2px solid #8f8f91; border-radius: 6px; background-color:red;}"
    "QLabel { font-size: 8pt; color: \"white\"; icon-size: 30px; } "
    "QToolBar { background: qlineargradient(x1: 0, y1: 0,    x2: 0, y2: 1, "
    "stop: 0 #66e, stop: 1 #bbf); background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1, "
    "stop: 0 #bbf, stop: 1 #55f) } ");


    QWidget *viewModeButtonsWidget = new QWidget;
    QVBoxLayout *viewModeLayout = new QVBoxLayout;

    viewModeSignalMapper = new QSignalMapper(this);

    for (int viewModeInt = ViewMode_BEGIN; viewModeInt != ViewMode_END; viewModeInt++)
    {
        // ViewMode viewMode = static_cast<ViewMode>(viewModeInt);
        QToolButton *button = new QToolButton(this);
        button->setAutoRaise(true);
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        viewModeButtons.insert(viewModeInt, button);
        button->addAction(viewActions.at(viewModeInt));
        button->setDefaultAction(viewActions.at(viewModeInt));
        viewModeLayout->addWidget(button);
    }

    QLabel *spacing = new QLabel(tr("____________"));
    viewModeLayout->addWidget(spacing);

    QToolButton *button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    viewModeLayout->addWidget(button);
    button->addAction(runAction);
    button->setDefaultAction(runAction);
    viewModeLayout->addWidget(button);

/*    QComboBox* parametersCB = new QComboBox;
    QLabel* modelLabel = new QLabel("IA");
    QLabel* trialLabel = new QLabel("masked_priming_ldt");
    QLabel* inputLabel = new QLabel("Davis_Lupker_06");
*/

//    vbox->addWidget(spacing);
//    vbox->addWidget(modelLabel);
//    vbox->addWidget(trialLabel);
//    vbox->addWidget(inputLabel);

    viewModeButtonsWidget->setLayout(viewModeLayout);
    infoToolBar->addWidget(viewModeButtonsWidget);
    addToolBar(Qt::LeftToolBarArea, infoToolBar);
}


void EasyNetMainWindow::loadFile(const QString &fileName)
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

void EasyNetMainWindow::setCurrentFile(EditWindow *window, const QString &fileName)
{
    window->setCurrentFile(fileName);
    window->textEdit->document()->setModified(false);
    setWindowModified(false);

    curJson = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName().append(".json"));
}

QString EasyNetMainWindow::strippedName(const QString &fullFileName)
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

void EasyNetMainWindow::hideAllDocks()
{
//    dockZeb->hide();
    dockWelcome->hide();
    dockWebWelcome->hide();
    dockEdit->hide();
    dockInterpreter->hide();
//    dockInput->hide();
    dockOutput->hide();
    dockExplorer->hide();
    dockDesignWindow->hide();
    dockCommandLog->hide();
}


void EasyNetMainWindow::showViewMode(int viewModeInt)
{
    hideAllDocks();
    switch (viewModeInt) {
    case Welcome:
        welcomeScreen->setUrl(QUrl("qrc:///images/Welcome.html"));
//        QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
        webWelcomeScreen->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
        webWelcomeScreen->setUrl(tr("http://www.adelmanlab.org/easyNet/"));

        dockWebWelcome->show();
        dockWelcome->show();
        break;
    case Model:
        dockDesignWindow->show();
        dockExplorer->show();
        break;
    case Trial:
        break;
    case Input:
        break;
    case Output:
        dockOutput->show();
        break;
    case Params:
        break;
    case Interpreter:
        dockInterpreter->show();
        break;
    case Code:
        dockEdit->show();
        dockCommandLog->show();
        dockInterpreter->show();
        break;
    default:
        break;
    }
}


void EasyNetMainWindow::run()
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


