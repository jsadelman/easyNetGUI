#include <QtWidgets>
#include <QtGlobal>
#include <QtDebug>
#include <QTreeView>
#include <QStringList>
#include <QEventLoop>
#include <QToolBar>
#include <QWebView>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>


#include <iostream>
#include <fstream>
#include <vector>

#include "easyNetMainWindow.h"
#include "treemodel.h"
#include "objexplorer.h"
#include "designwindow.h"
#include "codeeditor.h"
#include "sessionmanager.h"
#include "highlighter.h"
#include "editwindow.h"
#include "plotwindow.h"
#include "lazynutjobparam.h"
#include "lazynutjob.h"
#include "lazynutlistcombobox.h"
#include "objectcatalogue.h"
#include "objectcataloguefilter.h"
#include "lazynutconsole.h"
//#include "lazynutscripteditor.h"
#include "maxminpanel.h"
#include "tableeditor.h"


EasyNetMainWindow::EasyNetMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    test_gui = false;

    if (!test_gui)
        readSettings();

    if (!test_gui)
    {
        checkLazyNut();
        objectCatalogue = new ObjectCatalogue;
    }

    setWindowTitle(tr("easyNet"));
    setWindowIcon(QIcon(":/images/zebra.png"));
    setUnifiedTitleAndToolBarOnMac(true);

    createActions();
    createMenus();
    if (!test_gui)
        createStatusBar();

    if (!test_gui)
        initialiseToolBar();

    constructForms();
    createDockWindows();
    setCentralWidget(textEdit12);

    /* signals & slots */
    if (!test_gui)
    {
    connect(SessionManager::instance(), SIGNAL(recentlyCreated(QDomDocument*)),
            objectCatalogue, SLOT(create(QDomDocument*)));
    connect(SessionManager::instance(), SIGNAL(recentlyModified(QStringList)),
            objectCatalogue, SLOT(invalidateCache(QStringList)));
    connect(SessionManager::instance(), SIGNAL(recentlyDestroyed(QStringList)),
            objectCatalogue, SLOT(destroy(QStringList)));
    connect(SessionManager::instance(), SIGNAL(commandSent(QString)),
            commandLog, SLOT(addText(QString)));
    }

    // debug: load and run qtest at startup
//    loadFile(QString("%1/qtest").arg(scriptsDir));
//    run();

}

void EasyNetMainWindow::checkLazyNut()
{
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
        SessionManager::instance()->startLazyNut(lazyNutBat);
}

void EasyNetMainWindow::constructForms()
{
    // construct all of the forms
    textEdit1 = new QTextEdit;
    textEdit2 = new QTextEdit;
    textEdit3 = new QTextEdit;
    textEdit4 = new QTextEdit;
    textEdit5 = new QTextEdit;
    textEdit12 = new QTextEdit;
    textEdit12->hide();

    if (!test_gui)
    {
        lazyNutConsole = new LazyNutConsole(this);
        plotWindow = new PlotWindow(this);
        objExplorer = new ObjExplorer(objectCatalogue,this);
        designWindow = new DesignWindow(objectCatalogue, this);
        connect(this,SIGNAL(saveLayout()),
                designWindow,SIGNAL(saveLayout()));

    }

    scriptEdit = new EditWindow(this, newScriptAct, openAct, false);
    highlighter = new Highlighter(scriptEdit->textEdit->document());
    commandLog = new EditWindow(this, newLogAct, NULL, true); // no cut, no paste
    highlighter2 = new Highlighter(commandLog->textEdit->document());
    welcomeScreen = new QWebView(this);
    welcomeScreen->setUrl(QUrl("qrc:///images/Welcome.html"));
    stimSetForm = new TableEditor ("Stimuli",this);


    // construct the panels
    leftPanel = new QTabWidget;
    centrePanel = new QTabWidget;
    upperRightPanel = new QTabWidget;
    lowerRightPanel = new QTabWidget;

    if (!test_gui)
    {
        centrePanel->addTab(designWindow, tr("Model"));
        centrePanel->addTab(plotWindow, tr("Plots"));
        leftPanel->addTab(lazyNutConsole, tr("Console"));
        upperRightPanel->addTab(objExplorer, tr("Objects"));
//        upperRightPanel->addTab(textEdit5, tr("Objects")); // debug
    }
    centrePanel->addTab(textEdit1, tr("Trial"));
    leftPanel->addTab(commandLog, tr("History"));
    leftPanel->addTab(scriptEdit, tr("Script"));
    upperRightPanel->addTab(textEdit2, tr("Parameters"));
    upperRightPanel->addTab(stimSetForm, tr("Stimuli"));
    upperRightPanel->addTab(welcomeScreen, tr("Help"));
//    lowerRightPanel->addTab(plotWindow, tr("Plots"));
    lowerRightPanel->addTab(textEdit4, tr("Tables"));

    leftPanel->setMovable(true);
    // perhaps use this code for detachable tabs?
    // http://www.qtcentre.org/threads/61403-SOLVED-Detachable-QDockWidget-tabs

    upperRightPanel->setCurrentIndex(upperRightPanel->count() - 1); // start on Help tab, to welcome user
}

 void EasyNetMainWindow::createDockWindows()
{

    QDockWidget *dock = new QDockWidget("Explorer",this);
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    dock->setWidget(upperRightPanel);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
    dock->setFeatures(QDockWidget::DockWidgetClosable);

    dock = new QDockWidget(tr("Output"), this);
    dock->setWidget(lowerRightPanel);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
    dock->setFeatures(QDockWidget::DockWidgetClosable);

    codePanelDock = new MaxMinPanel(tr("lazyNut Code"),this);
    codePanelDock->setWidget(leftPanel);
    addDockWidget(Qt::LeftDockWidgetArea, codePanelDock);
    viewMenu->addAction(codePanelDock->toggleViewAction());
    codePanelDock->setFeatures(QDockWidget::DockWidgetClosable);
    codePanelDock->hide();

    QDockWidget *dock2 = new QDockWidget(tr("Visualiser"), this);
    dock2->setWidget(centrePanel);
    addDockWidget(Qt::LeftDockWidgetArea, dock2);
    viewMenu->addAction(dock2->toggleViewAction());
    dock2->setFeatures(QDockWidget::DockWidgetClosable);

}


void EasyNetMainWindow::initialiseToolBar()
{
//    QIcon *newpix = new QIcon(":/images/zebra_64x64.png");
//    QAction *newa = new QAction(newpix, "&New", this);
    toolbar = addToolBar("main toolbar");
//    QLabel* modelBoxLabel = new QLabel("Model: ");
    QPushButton* modelButton = new QPushButton("Model:");
    modelButton->setFlat(true);

//    QLabel* trialBoxLabel = new QLabel("Trial:");
    QPushButton* trialButton = new QPushButton("Trial:");
    trialButton->setFlat(true);

//    QLabel* setBoxLabel = new QLabel("Set:");
    QPushButton* setButton = new QPushButton("Set:");
    setButton->setFlat(true);

//    QLabel* inputBoxLabel = new QLabel("Input:");
    QPushButton* inputButton = new QPushButton("Input:");
    inputButton->setFlat(true);

    modelComboBox = new QComboBox(this);
    modelComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    trialComboBox = new QComboBox(this);
    trialComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    trialComboBox->setMinimumSize(100, trialComboBox->minimumHeight());

    setComboBox = new QComboBox;
    setComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(setComboBox,SIGNAL(currentIndexChanged(QString)),
            this, SLOT(updateStimuliView(QString)));

    inputComboBox = new QComboBox;
    inputComboBox->setEditable(true);
    inputComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
//    inputComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    inputComboBox->setMinimumSize(100, inputComboBox->minimumHeight());

    connect(modelButton, SIGNAL(clicked()),
              this, SLOT(loadModel()));
//    connect(trialButton, SIGNAL(clicked()),
//              this, SLOT(loadTrial()));

    connect(setButton, SIGNAL(clicked()), this, SLOT(loadStimulusSet()));



// want to catch Enter key press
//    connect(inputComboBox, SIGNAL(activated()),
//          this, SLOT(runTrial()));


//    connect(modelComboBox, SIGNAL(currentIndexChanged(QString)),

      modelListFilter = new ObjectCatalogueFilter(objectCatalogue, this);
      modelComboBox->setModel(modelListFilter);
      modelComboBox->setModelColumn(0);
//      modelComboBox->view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
      modelListFilter->setType("grouping");

      trialListFilter = new ObjectCatalogueFilter(objectCatalogue, this);
      trialComboBox->setModel(trialListFilter);
      trialComboBox->setModelColumn(0);
//      modelComboBox->view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
      trialListFilter->setType("steps");

    spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

//    toolbar->addWidget(modelBoxLabel);
    toolbar->addWidget(modelButton);
    // Add values in the combo box
    toolbar->addWidget(modelComboBox);
//    toolbar->addAction(openAct);
//    toolbar->addAction(QIcon(openpix), "Open File");
    toolbar->addSeparator();


//    toolbar->addWidget(trialBoxLabel);
    toolbar->addWidget(trialButton);
    // Add values in the combo box
    toolbar->addWidget(trialComboBox);
    toolbar->addSeparator();

//    toolbar->addWidget(setBoxLabel);
    toolbar->addWidget(setButton);
    // Add values in the combo box
    toolbar->addWidget(setComboBox);
    setComboBox->insertItem(0, "Untitled");
    toolbar->addSeparator();

//    toolbar->addWidget(inputBoxLabel);
    toolbar->addWidget(inputButton);
    // Add values in the combo box
    toolbar->addWidget(inputComboBox);
    inputComboBox->insertItem(0, "");
    toolbar->addSeparator();

    // toolBar is a pointer to an existing toolbar
    toolbar->addWidget(spacer);
    toolbar->addAction("Run",this, SLOT(runTrial()));
    toolbar->addSeparator();
    toolbar->addAction("Run all",this, SLOT(runAllTrial()));

}


void EasyNetMainWindow::updateStimuliView(QString text)
{
    if (!text.size())
        return;
    if (text=="Untitled")
        return;
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({QString("xml " + text + " get")});
    param->answerFormatterType = AnswerFormatterType::XML;
    param->setAnswerReceiver(stimSetForm, SLOT(addDataFrameToWidget(QDomDocument*)));
    SessionManager::instance()->setupJob(param, sender());
}

void EasyNetMainWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Choose a file name"), ".",
                        tr("HTML (*.html *.htm)"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Dock Widgets"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEdit1->toHtml();
    QApplication::restoreOverrideCursor();

    statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);
}


void EasyNetMainWindow::about()
{
   QMessageBox::about(this, tr("About Dock Widgets"),
            tr("The <b>Dock Widgets</b> example demonstrates how to "
               "use Qt's dock widgets. You can enter your own text, "
               "click a customer to add a customer name and "
               "address, and click standard paragraphs to add them."));
}

/*void EasyNetMainWindow::createActions()
{
    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save..."), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the current form letter"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}*/

/*void EasyNetMainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));

    viewMenu = menuBar()->addMenu(tr("&View"));

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}*/

void EasyNetMainWindow::runTrial()
{
    QString currentTrial = trialComboBox->currentText(); // "brief_masked";
    QString currentModel = modelComboBox->currentText(); // "iam";
    QString currentStimulus = inputComboBox->currentText(); // "mave";

    // check that above strings are in order
    if (currentTrial.isEmpty())
    {
        msgBox("Choose which type of trial to run");
        return;
    }
    if (currentModel.isEmpty())
    {
        msgBox("Choose which model to run");
        return;
    }
    if (currentStimulus.isEmpty())
    {
        msgBox("Specify the stimulus to run");
        return;
    }

    QString quietMode = "quietly ";
    QString stepCmd  = " step";
    QString modelArg = QString(" model=") + currentModel;
    QString stimArg = QString(" stimulus=") + currentStimulus;

    QString cmd = quietMode + currentTrial + stepCmd + modelArg + stimArg;
    SessionManager::instance()->runCmd(cmd);

//    after running cmd, call draw on plotForm
//     this seems like far too much code to achieve this !!
//    LazyNutJobParam *param = new LazyNutJobParam;
//    param->logMode |= ECHO_INTERPRETER;
//    param->cmdList = QStringList({cmd});
//    param->setNextJobReceiver(plotForm, SLOT(draw()));
//    SessionManager::instance()->setupJob(param);
}


void EasyNetMainWindow::runAllTrial()
{
    QString currentTrial = trialComboBox->currentText();
    QString currentModel = modelComboBox->currentText();
    QString stimulusSet = setComboBox->currentText();

    // check that above strings are in order
    if (currentTrial.isEmpty())
    {
        msgBox("Choose which type of trial to run");
        return;
    }
    if (currentModel.isEmpty())
    {
        msgBox("Choose which model to run");
        return;
    }
    if (stimulusSet.isEmpty())
    {
        msgBox("Specify the stimulus set to use");
        return;
    }

    QString quietMode = "quietly ";
    QString stepCmd  = " run_trials";
    QString modelArg = QString(" model=") + currentModel + QString(" ");
    QString stimArg = stimulusSet;

    QString cmd = quietMode + stimArg + stepCmd + modelArg + currentTrial;
    // andrews run_trials iam ldt
    SessionManager::instance()->runCmd(cmd);
}

void EasyNetMainWindow::msgBox(QString msg)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Debug", msg,
                                  QMessageBox::Yes|QMessageBox::No);

}

void EasyNetMainWindow::loadModel()
{
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load model"),
                                                    scriptsDir,
                                                    tr("Script Files (*.eNs *.eNm)"));
    if (!fileName.isEmpty())
    {
        // load and run script
        loadFile(fileName);

        // load JSON file, if it exists
        QFileInfo fi(fileName);
        QString base = fi.baseName();
        curJson = base.append(".json");
        qDebug() << "base =" << base << "; curJson = " << curJson;
        qDebug() << QFileInfo(fileName).dir();
//        qDebug() << QFileInfo(fileName).dir().filePath(QFileInfo(fileName));
        qDebug() << QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName());

        curJson = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName());
        qDebug() << curJson;
        curJson.append(".json");
        qDebug() << curJson;

        // set up signal - slots so that loadLayout will be called
        // once all of the layers/connections have descriptions
        designWindow->prepareToLoadLayout(curJson);

//        showViewMode(Model);
        run();
        // need to construct a job that'll run when model is loaded, i.e., lazyNut's ready
        // should then call getList() and choose the appropriate model

    }
}

void EasyNetMainWindow::loadStimulusSet()
{
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load stimulus set"),
                                                    stimDir,
                                                    tr("Database Files (*.eNd)"));
    if (!fileName.isEmpty())
    {
        // create db
        QFileInfo fi(fileName);
        QString base = fi.baseName();
        LazyNutJobParam *param = new LazyNutJobParam;
        param->logMode |= ECHO_INTERPRETER; // debug purpose
        param->cmdList = QStringList({
               QString("create stimulus_set %1").arg(base),
               QString("%1 load %2").arg(base).arg(fileName)
//                                         ,
//               QString("xml %1 get").arg(base)
                                     });
        param->answerFormatterType = AnswerFormatterType::XML;
        param->setAnswerReceiver(stimSetForm, SLOT(addDataFrameToWidget(QDomDocument*)));
        SessionManager::instance()->setupJob(param, sender());

        // change combobox text
        setComboBox->insertItem(0,base);
        setComboBox->setCurrentIndex(0);

        //showViewMode(Stimuli);
        upperRightPanel->setCurrentIndex(upperRightPanel->count() - 2); // show StimSet tab




    }
}

void EasyNetMainWindow::currentStimulusChanged(QString stim)
{
    if (inputComboBox->findText(stim) == -1)
        inputComboBox->addItem(stim);
    inputComboBox->setCurrentIndex(inputComboBox->findData(stim));


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

void EasyNetMainWindow::readSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    easyNetHome = settings.value("easyNetHome","").toString();
    lazyNutBat = settings.value("lazyNutBat","").toString();
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    scriptsDir = settings.value("scriptsDir", easyNetHome + "/Models").toString();
    QString defaultStimDir = easyNetHome + "/Databases/Stimulus_files";
    stimDir = defaultStimDir;
//    stimDir = settings.value("stimDir", defaultStimDir).toString();

//    resize(size);
    move(pos);

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();
}

void EasyNetMainWindow::writeSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    settings.setValue("easyNetHome", easyNetHome);
    settings.setValue("lazyNutBat",lazyNutBat);
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("scriptsDir",scriptsDir);
    settings.setValue("stimDir",stimDir);

    settings.beginGroup("mainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();
}



void EasyNetMainWindow::closeEvent(QCloseEvent *event)
{
        writeSettings();
        SessionManager::instance()->killLazyNut();
        emit saveLayout();
        event->accept();
}

void EasyNetMainWindow::checkScreens()
{
    foreach (QScreen *screen, QGuiApplication::screens()) {
        qDebug() << "Information for screen:" << screen->name();
        qDebug() << "  Available geometry:" << screen->availableGeometry().x() << screen->availableGeometry().y() << screen->availableGeometry().width() << "x" << screen->availableGeometry().height();
        qDebug() << "  Available size:" << screen->availableSize().width() << "x" << screen->availableSize().height();
        qDebug() << "  Available virtual geometry:" << screen->availableVirtualGeometry().x() << screen->availableVirtualGeometry().y() << screen->availableVirtualGeometry().width() << "x" << screen->availableVirtualGeometry().height();
        qDebug() << "  Available virtual size:" << screen->availableVirtualSize().width() << "x" << screen->availableVirtualSize().height();
        qDebug() << "  Depth:" << screen->depth() << "bits";
        qDebug() << "  Geometry:" << screen->geometry().x() << screen->geometry().y() << screen->geometry().width() << "x" << screen->geometry().height();
        qDebug() << "  Logical DPI:" << screen->logicalDotsPerInch();
        qDebug() << "  Logical DPI X:" << screen->logicalDotsPerInchX();
        qDebug() << "  Logical DPI Y:" << screen->logicalDotsPerInchY();
        qDebug() << "  Physical DPI:" << screen->physicalDotsPerInch();
        qDebug() << "  Physical DPI X:" << screen->physicalDotsPerInchX();
        qDebug() << "  Physical DPI Y:" << screen->physicalDotsPerInchY();
        qDebug() << "  Physical size:" << screen->physicalSize().width() << "x" << screen->physicalSize().height() << "mm";
        qDebug() << "  Refresh rate:" << screen->refreshRate() << "Hz";
        qDebug() << "  Size:" << screen->size().width() << "x" << screen->size().height();
        qDebug() << "  Virtual geometry:" << screen->virtualGeometry().x() << screen->virtualGeometry().y() << screen->virtualGeometry().width() << "x" << screen->virtualGeometry().height();
        qDebug() << "  Virtual size:" << screen->virtualSize().width() << "x" << screen->virtualSize().height();
    }
}

void EasyNetMainWindow::setCurrentFile(EditWindow *window, const QString &fileName)
{
    window->setCurrentFile(fileName);
    window->textEdit->document()->setModified(false);
    setWindowModified(false);

    curJson = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName().append(".json"));
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
//            showViewMode(Code);
        }
 //   }
}


void EasyNetMainWindow::runModel()
{
    runCmdAndUpdate(scriptEdit->textEdit->getAllText());
}

void EasyNetMainWindow::runSelection()
{
    runCmdAndUpdate(scriptEdit->textEdit->getSelectedText());
}

//! [runCmdAndUpdate]
void EasyNetMainWindow::runCmdAndUpdate(QStringList cmdList)
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER;
    param->cmdList = cmdList;
    param->setNextJobReceiver(SessionManager::instance(), SLOT(updateObjectCatalogue()));
    SessionManager::instance()->setupJob(param);
}
//! [runCmdAndUpdate]

//! [getVersion]
void EasyNetMainWindow::getVersion()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER;
    param->cmdList = QStringList({"version"});
    param->answerFormatterType = AnswerFormatterType::Identity;
    param->setAnswerReceiver(this, SLOT(displayVersion(QString)));
    SessionManager::instance()->setupJob(param);
}
//! [getVersion]



void EasyNetMainWindow::echoCommand(const QString &line)
{
//    QString return_line = runCmd(line);
    SessionManager::instance()->runCmd(line);
//    if (return_line.size() > 1)
//    commandLog->textEdit->insertPlainText(return_line);
        commandLog->textEdit->insertPlainText(line);
}

void EasyNetMainWindow::setEasyNetHome()
{
    easyNetHome = QFileDialog::getExistingDirectory(this,tr("Please select your easyNet home directory.\n"));
    lazyNutBat = easyNetHome + QString("/%1/nm_files/%2").arg(binDir).arg(lazyNutBasename);
    SessionManager::instance()->startLazyNut(lazyNutBat);
}

void EasyNetMainWindow::setLazyNutBat()
{
    lazyNutBat = QFileDialog::getOpenFileName(this,QString(tr("Please select your %1 file.")).arg(lazyNutBasename),
                                              easyNetHome,QString("*.%1").arg(lazyNutExt));
    SessionManager::instance()->startLazyNut(lazyNutBat);
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
//    createViewActions();

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
    loadModelAct = new QAction(QIcon(":/images/open.png"), tr("&Load model"), this);
    loadModelAct->setShortcuts(QKeySequence::Open);
    loadModelAct->setStatusTip(tr("Load a previously specified model"));
    connect(loadModelAct, SIGNAL(triggered()), this, SLOT(loadModel()));

    loadStimulusSetAct = new QAction(tr("Load &stimulus set"), this);
    loadStimulusSetAct->setStatusTip(tr("Load a stimulus set"));
    connect(loadStimulusSetAct, SIGNAL(triggered()), this, SLOT(loadStimulusSet()));

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
    connect(stopAct,SIGNAL(triggered()),SessionManager::instance(),SLOT(stop()));
    pauseAct = new QAction("PAUSE",this);
    connect(pauseAct,SIGNAL(triggered()),SessionManager::instance(),SLOT(pause()));
    connect(SessionManager::instance(),SIGNAL(isPaused(bool)),this,SLOT(showPauseState(bool)));

    versionAct = new QAction("Version",this);
    connect(versionAct,SIGNAL(triggered()),this,SLOT(getVersion()));

}

void EasyNetMainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    //fileMenu->addAction(newAct);
    fileMenu->addAction(loadModelAct);
 //   fileMenu->addAction(openAct);
    //fileMenu->addAction(saveAct);
    //fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(loadStimulusSetAct);
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

    viewMenu = menuBar()->addMenu(tr("&View"));
    menuBar()->addSeparator();

    aboutMenu = menuBar()->addMenu(tr("&About"));
    aboutMenu->addAction(versionAct);
}


/******** StatusBar code *********/

void EasyNetMainWindow::createStatusBar()
{
    lazyNutProgressBar = new QProgressBar;
    lazyNutProgressBar->setTextVisible(false);
    lazyNutProgressBar->setMinimum(0);
    statusBar()->addPermanentWidget(lazyNutProgressBar, 1);
    connect(SessionManager::instance(), SIGNAL(commandsInJob(int)),
            lazyNutProgressBar, SLOT(setMaximum(int)));
    connect(SessionManager::instance(), SIGNAL(commandExecuted(QString)),
            this, SLOT(addOneToLazyNutProgressBar()));


    readyLabel = new QLabel("READY");
    readyLabel->setStyleSheet("QLabel {"
                              "qproperty-alignment: AlignCenter;"
                              "padding-right: 3px;"
                              "padding-left: 3px;"
                              "font-weight: bold;"
                              "color: green;"
                              "}");
    busyLabel = new QLabel("BUSY");
    busyLabel->setStyleSheet("QLabel {"
                             "qproperty-alignment: AlignCenter;"
                             "padding-right: 3px;"
                             "padding-left: 3px;"
                             "font-weight: bold;"
                             "background-color : red;"
                             "color: white;"
                             "}");
    offLabel = new QLabel("OFF");
    offLabel->setStyleSheet("QLabel {"
                            "qproperty-alignment: AlignCenter;"
                            "padding-right: 3px;"
                            "padding-left: 3px;"
                            "font-weight: bold;"
                            "background-color: black;"
                            "color: yellow;"
                            "}");
    lazyNutStatusWidget = new QStackedWidget;
    lazyNutStatusWidget->addWidget(readyLabel);
    lazyNutStatusWidget->addWidget(busyLabel);
    lazyNutStatusWidget->addWidget(offLabel);
    lazyNutStatusWidget->setCurrentWidget(offLabel);
    lazyNutStatusWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lazyNutStatusWidget->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBar()->addPermanentWidget(lazyNutStatusWidget, 1);
    connect(SessionManager::instance(), SIGNAL(isReady(bool)), this, SLOT(setLazyNutIsReady(bool)));





    lazyNutCmdLabel = new QLabel;
    lazyNutCmdLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBar()->addWidget(lazyNutCmdLabel, 1);
    connect(SessionManager::instance(), SIGNAL(commandExecuted(QString)),
            this, SLOT(showCmdOnStatusBar(QString)));

    lazyNutErrorLabel = new QLabel;
    lazyNutErrorLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    lazyNutErrorLabel->setStyleSheet("QLabel {"
                                 "font-weight: bold;"
                                 "color: red"
                                 "}");
    statusBar()->addWidget(lazyNutErrorLabel, 1);
    connect(SessionManager::instance(), SIGNAL(cmdError(QString,QStringList)),
            this, SLOT(showErrorOnStatusBar(QString,QStringList)));
    connect(SessionManager::instance(), SIGNAL(lazyNutMacroStarted()),
            this, SLOT(clearErrorOnStatusBar()));
}


void EasyNetMainWindow::setLazyNutIsReady(bool isReady)
{
    if (isReady)
    {
        lazyNutStatusWidget->setCurrentWidget(readyLabel);
        lazyNutProgressBar->reset();
    }
    else
    {
        lazyNutStatusWidget->setCurrentWidget(busyLabel);
        lazyNutProgressBar->setValue(0);
    }
}

void EasyNetMainWindow::showErrorOnStatusBar(QString /*cmd*/, QStringList errorList)
{
    lazyNutErrorLabel->setText(QString("LAST ERROR: %1").arg(errorList.last()));
}

void EasyNetMainWindow::clearErrorOnStatusBar()
{
    lazyNutErrorLabel->clear();
}

void EasyNetMainWindow::showCmdOnStatusBar(QString cmd)
{
    lazyNutCmdLabel->setText(QString("LAST EXEC COMMAND: %1").arg(cmd));
}

void EasyNetMainWindow::addOneToLazyNutProgressBar()
{
    lazyNutProgressBar->setValue(lazyNutProgressBar->value()+1);
}


/*********************************************************
 * old view menu stuff here
 *
 */

/*
void EasyNetMainWindow::createToolBars()
{
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



//    vbox->addWidget(spacing);
//    vbox->addWidget(modelLabel);
//    vbox->addWidget(trialLabel);
//    vbox->addWidget(inputLabel);

    viewModeButtonsWidget->setLayout(viewModeLayout);
    infoToolBar->addWidget(viewModeButtonsWidget);
    addToolBar(Qt::LeftToolBarArea, infoToolBar);
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

void EasyNetMainWindow::hideAllDocks()
{
    dockWelcome->hide();
//    dockWebWelcome->hide();
    dockEdit->hide();
    dockInterpreter->hide();
//    dockInput->hide();
    dockOutput->hide();
    dockExplorer->hide();
    dockDesignWindow->hide();
    dockCommandLog->hide();
    statusBar()->show();

}


void EasyNetMainWindow::showViewMode(int viewModeInt)
{
    hideAllDocks();
    switch (viewModeInt) {
    case Welcome:
        welcomeScreen->setUrl(QUrl("qrc:///images/Welcome.html"));
//        QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
//        webWelcomeScreen->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
//        webWelcomeScreen->setUrl(tr("http://www.adelmanlab.org/easyNet/"));

//        dockWebWelcome->show();
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

    statusBar()->show();
//    showNormal();
//    this->showMinimized();
//    this->showMaximized();
}
*/

void EasyNetMainWindow::run()
{
    runModel(); // ultimately this will have different action depending on which mode is active

}


