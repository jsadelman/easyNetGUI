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
#include <QtCore/QLibraryInfo>
#include <QMessageBox>
#include <QDir>
#include <QFontMetrics>
#include <QFont>


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
#include "objectcache.h"
#include "objectcachefilter.h"
#include "lazynutconsole.h"
//#include "lazynutscripteditor.h"
#include "maxminpanel.h"
#include "tableeditor.h"
#include "findfiledialog.h"
#include "assistant.h"
#include "textedit.h"
#include "helpwindow.h"
#include "trialwidget.h"
#include "trialeditor.h"
#include "commandlog.h"
#include "scripteditor.h"
#include "console.h"
#include "debuglog.h"
#include "plotviewer.h"
#include "diagramscenetabwidget.h"
#include "diagramscene.h"
#include "diagramwindow.h"
#include "tableviewer2.h"
#include "trialdataframemodel.h"
#include "enumclasses.h"


MainWindow* MainWindow::mainWindow = nullptr;

MainWindow* MainWindow::instance()
{
    return mainWindow ? mainWindow : (mainWindow = new MainWindow);
}

MainWindow::MainWindow(QWidget *parent)
    : easyNetHome(""), easyNetDataHome(""), QMainWindow(parent)
{


    readSettings();
    setDefaultLocations();
    SessionManager::instance()->startLazyNut(lazyNutBat);

//    checkLazyNut();



    setWindowTitle(tr("easyNet"));
#ifdef __APPLE__
    setWindowIcon(QIcon(":/images/easyNet.hqx"));
#else
    setWindowIcon(QIcon(":/images/zebra.png"));
#endif
    setUnifiedTitleAndToolBarOnMac(true);

    createActions();

    createMenus();


    initialiseToolBar(); // this constructs the trialWidget

    constructForms();
    createDockWindows();

    setFontSize("medium");
    setMediumFontAct->setChecked(true);
    createStatusBar();

    QWidget *dummyWidget = new QWidget;
    dummyWidget->hide();
    setCentralWidget(dummyWidget);

    connectSignalsAndSlots();

    // debug: load and run qtest at startup
//    loadFile(QString("%1/qtest").arg(scriptsDir));
//    runScript();

    /* INITIAL DISPLAY AT STARTUP */
//    showExplorer();
    introDock->raise();
    introPanel->setCurrentIndex(infoTabIdx); // start on Intro tab, to welcome user
    diagramDock->raise();
    diagramPanel->setCurrentIndex(modelTabIdx);
    diagramSceneTabChanged(modelTabIdx);
    diagramWindow->ToggleControlsDock(); // hide layout controls
    setQuietMode();



    #ifdef WIN32
    if (qApp->arguments().count() > 1)
        loadModel(QDir::fromNativeSeparators(qApp->arguments().at(1)));
    #endif
}

void MainWindow::checkLazyNut()
{
    connect(SessionManager::instance(), SIGNAL(lazyNutNotRunning()),
            this, SLOT(lazyNutNotRunning()));
//    if (easyNetHome.isEmpty())
//    {
//        QMessageBox::warning(this, "warning",QString("Please select "
//                                                     "a valid easyNet home directory using"
//                                                     "the menu Settings -> Set easyNet home directory"));
//    }
//    else
//    {
        lazyNutBat = QString("%1/%2/nm_files/%3").arg(easyNetHome).arg(binDir).arg(lazyNutBasename);
//    }
    if (!lazyNutBat.isEmpty())
        SessionManager::instance()->startLazyNut(lazyNutBat);
}

void MainWindow::constructForms()
{
    /* CONSTRUCT TABWIDGETS */

    introPanel = new QTabWidget;
    lazynutPanel = new QTabWidget;
    methodsPanel = new QTabWidget;
    explorerPanel = new QTabWidget;
    resultsPanel = new QTabWidget;
    diagramPanel = new DiagramSceneTabWidget(this);
    lazynutPanel->setMovable(true);

    /* CONSTRUCT ALL THE INDIVIDUAL FORMS */

    lazyNutConsole2 = new Console(this);

    plotSettingsWindow = new PlotSettingsWindow(this);
    objExplorer = new ObjExplorer(SessionManager::instance()->descriptionCache,this);
    scriptEdit = new ScriptEditor(scriptsDir, this);
    highlighter = new Highlighter(scriptEdit->textEdit->document());
//    commandLog = new EditWindow(this, newLogAct, loadScriptAct, true); // no cut, no paste
    commandLog = new CommandLog(this);
    highlighter2 = new Highlighter(commandLog->textEdit->document());
    errorLog = new CommandLog(this);
    highlighter3 = new Highlighter(errorLog->textEdit->document());
    debugLog = new DebugLog (this);
//    welcomeScreen = new QWebView(this);
//    welcomeScreen->setUrl(QUrl("qrc:///images/Welcome.html"));
    stimSetForm = new TableEditor ("Stimuli",this);
//    tablesWindow = new TableEditor (SessionManager::instance()->descriptionCache,"Tables",this);
    tablesWindow = new TableViewer("Tables",this);
    dataframesWindow = new TableEditor (SessionManager::instance()->descriptionCache,"Dataframes",this);
    paramEdit = new TableEditor ("Parameters",this);
    plotViewer = new PlotViewer(easyNetHome, this);
    diagramWindow = new DiagramWindow(diagramPanel, this);
    trialEditor = new TrialEditor(this);

    infoWindow = new HelpWindow;
    assistant = new Assistant(easyNetDataHome + "/documentation/easyNetDemo.qhc");
    infoWindow->show();


    /* ADD TABS */
    infoTabIdx = introPanel->addTab(infoWindow, tr("Intro"));
    modelTabIdx = diagramPanel->newDiagramScene(tr("Model"), "layer", "connection");
    conversionTabIdx = diagramPanel->newDiagramScene(tr("Conversions"), "representation", "conversion");
    modelScene = diagramPanel->diagramSceneAt(modelTabIdx);
    conversionScene = diagramPanel->diagramSceneAt(conversionTabIdx);

    stimSetTabIdx = methodsPanel->addTab(stimSetForm, tr("Stimuli"));
    trialFormTabIdx = methodsPanel->addTab(trialEditor, tr("Trial")); //textEdit1
    paramTabIdx = methodsPanel->addTab(paramEdit, tr("Parameters"));
    plotSettingsTabIdx = methodsPanel->addTab(plotSettingsWindow, tr("Plot settings"));

    lazynutPanel->addTab(lazyNutConsole2, tr("Console"));
    lazynutPanel->addTab(commandLog, tr("History"));
    lazynutPanel->addTab(errorLog, tr("Errors"));
    scriptTabIdx = lazynutPanel->addTab(scriptEdit, tr("Script"));
    lazynutPanel->addTab(debugLog, tr("Debug log"));

//    infoTabIdx = explorerPanel->addTab(infoWindow, tr("Info"));
    explorerTabIdx = explorerPanel->addTab(objExplorer, tr("Objects"));
    dfTabIdx = explorerPanel->addTab(dataframesWindow, tr("Dataframes"));

    plotTabIdx = resultsPanel->addTab(plotViewer, tr("Plots"));
    outputTablesTabIdx = resultsPanel->addTab(tablesWindow, tr("Tables"));

    // perhaps use this code for detachable tabs?
    // http://www.qtcentre.org/threads/61403-SOLVED-Detachable-QDockWidget-tabs
}

void MainWindow::connectSignalsAndSlots()
{
    // refresh params when user clicks on param tab or changes model in combobox
    connect(explorerPanel, SIGNAL(currentChanged(int)),this,SLOT(explorerTabChanged(int)));
    connect(this,SIGNAL(paramTabEntered(QString)),paramEdit,SLOT(updateParamTable(QString)));
    connect(modelComboBox, SIGNAL(currentIndexChanged(QString)),paramEdit,SLOT(updateParamTable(QString)));
    connect(modelComboBox, SIGNAL(currentIndexChanged(QString)),
            SessionManager::instance(), SLOT(setCurrentModel(QString)));
    connect(this,SIGNAL(newTableSelection(QString)),tablesWindow,SLOT(updateTableView(QString)));
    connect(paramEdit, SIGNAL(newParamValueSig(QString,QString)),
            this,SLOT(setParam(QString,QString)));
    connect(plotSettingsWindow, SIGNAL(plot(QString,QByteArray)),
            plotViewer,SLOT(loadByteArray(QString,QByteArray)));
    connect(plotSettingsWindow, SIGNAL(newPlotSignal(QString)),
            plotViewer,SLOT(addPlot(QString)));

    connect(plotViewer,SIGNAL(sendDrawCmd(QString)),plotSettingsWindow,SLOT(sendDrawCmd(QString)));
    connect(plotViewer,SIGNAL(resized(QSize)),plotSettingsWindow,SLOT(newAspectRatio(QSize)));
    connect(plotViewer,SIGNAL(showPlotSettings()),this,SLOT(showPlotSettings()));
    connect(tablesWindow,SIGNAL(showPlotSettings()),this,SLOT(showPlotSettings()));
    connect(plotViewer,SIGNAL(setPlot(QString)), plotSettingsWindow, SLOT(setPlot(QString)));
//    connect(plotViewer,SIGNAL(hidePlotSettings()), plotSettingsWindow, SLOT(hidePlotSettings()));
    connect(plotSettingsWindow,SIGNAL(showPlotViewer()), this, SLOT(showPlotViewer()));
    connect(stimSetForm, SIGNAL(columnDropped(QString)),trialWidget,SLOT(showSetLabel(QString)));
    connect(stimSetForm, SIGNAL(restoreComboBoxText()),trialWidget,SLOT(restoreComboBoxText()));
    connect(stimSetForm, SIGNAL(openFileRequest()),this,SLOT(loadStimulusSet()));
    connect(dataframesWindow, SIGNAL(openFileRequest()),this,SLOT(importDataFrame()));
//    connect(diagramPanel, SIGNAL(currentDiagramSceneChanged(DiagramScene*)),
//            this, SLOT(diagramSceneTabChanged(DiagramScene*)));
    connect(diagramPanel, SIGNAL(currentChanged(int)), this, SLOT(diagramSceneTabChanged(int)));
//    connect(trialWidget,SIGNAL(runAllModeChanged(bool)),this,SLOT(setRunAllMode(bool)));
    connect(scriptEdit,SIGNAL(runCmdAndUpdate(QStringList)),this,SLOT(runCmdAndUpdate(QStringList)));
    connect(SessionManager::instance(),SIGNAL(userLazyNutOutputReady(QString)),
            lazyNutConsole2,SLOT(addText(QString)));
    connect(lazyNutConsole2,SIGNAL(historyKey(int)),
            this,SLOT(processHistoryKey(int)));
    connect(this,SIGNAL(showHistory(QString)),
            lazyNutConsole2,SLOT(showHistory(QString)));
    connect(trialComboBox,SIGNAL(currentIndexChanged(QString)),
            trialEditor,SLOT(setTrialName(QString)));
    connect(trialComboBox,SIGNAL(currentIndexChanged(QString)),
            SessionManager::instance(), SLOT(setCurrentTrial(QString)));
    connect(trialComboBox,SIGNAL(currentIndexChanged(QString)),
            tablesWindow,SLOT(addTrialTable(QString)));
    connect(modelScene,SIGNAL(objectSelected(QString)), objExplorer,SIGNAL(objectSelected(QString)));
    connect(modelScene,SIGNAL(objectSelected(QString)), this,SLOT(showExplorer()));
    connect(modelScene,SIGNAL(createNewPlotOfType(QString,QString,QMap<QString,QString>)),
            plotSettingsWindow,SLOT(createNewPlotOfType(QString,QString,QMap<QString,QString>)));
    connect(modelScene, SIGNAL(plotDestroyed(QString)), plotSettingsWindow, SLOT(removePlot(QString)));
    connect(modelScene, SIGNAL(plotDestroyed(QString)), plotViewer, SLOT(makeSnapshot(QString)));
//    connect(modelScene,SIGNAL(createNewPlotOfType(QString,QString,QMap<QString,QString>)),
//            this,SLOT(showPlotViewer()));
    connect(tablesWindow, SIGNAL(createNewPlotOfType(QString, QString, QMap<QString, QString>)),
            plotSettingsWindow, SLOT(createNewPlotOfType(QString, QString, QMap<QString, QString>)));
    connect(conversionScene,SIGNAL(objectSelected(QString)), objExplorer,SIGNAL(objectSelected(QString)));
    connect(conversionScene,SIGNAL(objectSelected(QString)), this,SLOT(showExplorer()));
    /* signals & slots */
//    connect(SessionManager::instance(), SIGNAL(recentlyCreated(QDomDocument*)),
//          SessionManager::instance()->descriptionCache(), SLOT(create(QDomDocument*)));
//    connect(SessionManager::instance(), SIGNAL(recentlyModified(QStringList)),
//          SessionManager::instance()->descriptionCache(), SLOT(invalidateCache(QStringList)));
//    connect(SessionManager::instance(), SIGNAL(recentlyDestroyed(QStringList)),
//          SessionManager::instance()->descriptionCache(), SLOT(destroy(QStringList)));
    connect(SessionManager::instance(), SIGNAL(logCommand(QString)),
            commandLog, SLOT(addText(QString)));
    connect(SessionManager::instance(), SIGNAL(commandExecuted(QString,QString)),
            debugLog, SLOT(addRowToTable(QString,QString)));
    connect(SessionManager::instance(), &SessionManager::cmdError, [=](QString /*cmd*/, QStringList errorList)
    {
       foreach(QString error, errorList)
           errorLog->addText(error);
    });

}

void MainWindow::showExplorer()
{
    explorerDock->show();
    explorerDock->setFocus();
    explorerDock->raise();
    explorerPanel->setCurrentIndex(explorerTabIdx);
}

void MainWindow::showPlotSettings()
{
    methodsDock->raise();
    methodsPanel->setCurrentIndex(plotSettingsTabIdx);
//    plotSettingsWindow->setPlot(plotName);
}

void MainWindow::showPlotViewer()
{
    resultsDock->raise();
    resultsPanel->setCurrentIndex(plotTabIdx);
}


void MainWindow::diagramSceneTabChanged(int index)
{
     modelScene->goToSleep();
     conversionScene->goToSleep();
    if (index == modelTabIdx)
         modelScene->wakeUp();
    else if (index == conversionTabIdx)
         conversionScene->wakeUp();
}


void MainWindow::setParam(QString paramDataFrame, QString newParamValue)
{
    QString cmd1 = paramDataFrame + " set " + newParamValue;
    QString cmd2 = QString("xml ") + paramDataFrame + " get ";
    SessionManager::instance()->runCmd({cmd1,cmd2});
}

void MainWindow::explorerTabChanged(int idx)
{
//    qDebug() << "Entered explorerTabChanged():" << idx;
    if (modelComboBox->currentText().isEmpty())
        return;
    if (idx == paramTabIdx)
            emit (paramTabEntered(modelComboBox->currentText()));

}

 void MainWindow::createDockWindows()
{

     introDock = new QDockWidget("Intro",this);
     introDock->setWidget(introPanel);
     addDockWidget(Qt::LeftDockWidgetArea, introDock);
     viewMenu->addAction(introDock->toggleViewAction());
     introDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

     codePanelDock = new QDockWidget(tr("lazyNut Code"),this);
     codePanelDock->setWidget(lazynutPanel);
     addDockWidget(Qt::LeftDockWidgetArea, codePanelDock);
     viewMenu->addAction(codePanelDock->toggleViewAction());
     codePanelDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
 //    codePanelDock->hide(); // initially, don't show codePanelDock

     methodsDock = new QDockWidget(tr("Method"), this);
     methodsDock->setWidget(methodsPanel);
     addDockWidget(Qt::LeftDockWidgetArea, methodsDock);
     viewMenu->addAction(methodsDock->toggleViewAction());
     methodsDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);


     diagramDock = new QDockWidget(tr("Architecture"), this);
     diagramDock->setWidget(diagramWindow);
     addDockWidget(Qt::RightDockWidgetArea, diagramDock);
     viewMenu->addAction(diagramDock->toggleViewAction());
     diagramDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

     explorerDock = new QDockWidget("Explorer",this);
     explorerDock->setWidget(explorerPanel);
     addDockWidget(Qt::RightDockWidgetArea, explorerDock);
     viewMenu->addAction(explorerDock->toggleViewAction());
     explorerDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

     resultsDock = new QDockWidget(tr("Results"), this);
     resultsDock->setWidget(resultsPanel);
     addDockWidget(Qt::RightDockWidgetArea, resultsDock);
     viewMenu->addAction(resultsDock->toggleViewAction());
     resultsDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

     tabifyDockWidget(introDock, codePanelDock);
     tabifyDockWidget(codePanelDock, methodsDock);
     tabifyDockWidget(diagramDock, explorerDock);
     tabifyDockWidget(explorerDock, resultsDock);
}


void MainWindow::initialiseToolBar()
{
//    QIcon *newpix = new QIcon(":/images/zebra_64x64.png");
//    QAction *newa = new QAction(newpix, "&New", this);
    toolbar = addToolBar("main toolbar");
//    QLabel* modelBoxLabel = new QLabel("Model: ");
    modelButton = new QPushButton("Model:");
    modelButton->setFlat(true);
    modelButton->setEnabled(true);

//    QLabel* trialBoxLabel = new QLabel("Trial:");
    trialButton = new QPushButton("Trial:");
    trialButton->setFlat(true);
    trialButton->setEnabled(false);

    modelComboBox = new QComboBox(this);
    modelComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    trialComboBox = new QComboBox(this);
    trialComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    trialComboBox->setMinimumSize(100, trialComboBox->minimumHeight());

    connect(modelButton, SIGNAL(clicked()),
              this, SLOT(loadModel()));
    connect(trialButton, SIGNAL(clicked()),
              this, SLOT(loadTrial()));

      modelListFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
      modelComboBox->setModel(modelListFilter);
      modelComboBox->setModelColumn(0);
//      modelComboBox->view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
      modelListFilter->setType("grouping");


      trialListFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
      trialComboBox->setModel(trialListFilter);
      trialComboBox->setModelColumn(0);
//      modelComboBox->view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
      trialListFilter->setType("steps");
      connect(trialListFilter, SIGNAL(objectCreated(QString,QString,QDomDocument*)),
              trialComboBox, SLOT(setCurrentText(QString)));



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

    trialWidget = new TrialWidget(this);
    toolbar->addWidget(trialWidget);
    connect(trialComboBox,SIGNAL(currentIndexChanged(QString)),trialWidget,SLOT(update(QString)));

    toolbar->addSeparator();

    // toolBar is a pointer to an existing toolbar
    toolbar->addWidget(spacer);
    toolbar->addSeparator();

    // notifier of run all trial
    QLabel *runAllTrialLabel = new QLabel("Processing a set of trials\nPlease wait...");
    runAllTrialLabel->setStyleSheet("QLabel {"
                                "color: red;"
                                 "border: 1px solid red;"
                                 "padding: 4px;"
                                 "font: bold 12pt;"
                                 "}");
    runAllTrialMsgAct = toolbar->addWidget(runAllTrialLabel);
    runAllTrialMsgAct->setVisible(false);
    connect(this, &MainWindow::runAllTrialEnded, [=]()
    {
        runAllTrialMsgAct->setVisible(false);
    });

}

void MainWindow::updateTableView(QString text)
{
    qDebug() << "Entered EasyNetMainWindow updateTableView with " << text;
    if (!text.size())
        return;
    if (text=="Untitled")
        return;

    TableEditor *table = dynamic_cast<TableEditor*> (sender());
    qDebug() << "sender is " << sender();
    qDebug() << "table is " << table;
    if( table == NULL)
        table = stimSetForm;
    qDebug() << "table is " << table;

    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("xml " + text + " get")});
    job->setAnswerReceiver(table, SLOT(addDataFrameToWidget(QDomDocument*)), AnswerFormatterType::XML);
    SessionManager::instance()->submitJobs(job);
}


void MainWindow::setQuietMode()
{
    if (setQuietModeAct->isChecked())
        quietMode = "quietly ";
    else
        quietMode = "";
}

void MainWindow::msgBox(QString msg)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Debug", msg,
                                  QMessageBox::Yes|QMessageBox::No);

}

void MainWindow::runScript()
{
    scriptEdit->runScript();
}

void MainWindow::loadModel(QString fileName)
{
    if (fileName.isEmpty())
        return;
#ifdef WIN32
    if (!SessionManager::instance()->currentModel().isEmpty())
    {
        QProcess::startDetached(
                    QString("%1/easyNet.bat").arg(qApp->applicationDirPath()),
                    QStringList({fileName}),
                    qApp->applicationDirPath());
        return;
    }
#endif
    modelScene->goToSleep();
    conversionScene->goToSleep();

    // load and run script
    loadFile(fileName);

    // the /path/basename is used by DiagramScene objects to load JSON files
    QString base = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName());
    modelScene->setBaseName(base);
    conversionScene->setBaseName(base);
    setWindowTitle(QFileInfo(fileName).completeBaseName());


    // set up signal - slots so that loadLayout will be called
    // once all of the layers/connections have descriptions
    //        designWindow->prepareToLoadLayout(base);
    //        conversionWindow->prepareToLoadLayout(base);

    // show info page, if there is one
    QString page = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName());
    page.append(".html");
    qDebug() << "page = " << page;
    if (QFileInfo(page).exists())
        infoWindow->showInfo(page);

    // note: this synch is wrong, yet it works fine if one loads just one model while
    // no other jobs run. In general commandsCompleted() might be sent from a previous job.
    connect(SessionManager::instance(),SIGNAL(commandsCompleted()),this,SLOT(afterModelLoaded()));
    runScript();
    //        SessionManager::instance()->setCurrentModel(currentModel);
    // need to construct a job that'll run when model is loaded, i.e., lazyNut's ready
    // should then call getList() and choose the appropriate model

    trialButton->setEnabled(true);
    loadAddOnAct->setEnabled(true);

    #ifndef WIN32
            loadModelAct->setEnabled(false);
            modelButton->setEnabled(false);
    #endif


}

void MainWindow::loadModel()
{
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load model"),
                                                    scriptsDir,
                                                    tr("easyNet Model Files (*.eNm)"));
    loadModel(fileName);
}

void MainWindow::afterModelLoaded()
{
    modelScene->setNewModelLoaded(true);
    conversionScene->setNewModelLoaded(true);
    diagramSceneTabChanged(diagramPanel->currentIndex());
//    modelScene->wakeUp();
    disconnect(SessionManager::instance(),SIGNAL(commandsCompleted()),this,SLOT(afterModelLoaded()));
}


void MainWindow::loadTrial()
{
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load trial"),
                                                    trialsDir,
                                                    tr("Script Files (*.eNs *.eNt)"));
    if (!fileName.isEmpty())
    {
        // load and run script
        loadFile(fileName);

//        // load JSON file, if it exists
//        QFileInfo fi(fileName);
//        QString base = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName());

//        // set up signal - slots so that loadLayout will be called
//        // once all of the layers/connections have descriptions
//        designWindow->prepareToLoadLayout(base);
//        conversionWindow->prepareToLoadLayout(base);

//        // show info page, if there is one
//        QString page = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName());
//        page.append(".html");
//        qDebug() << "page = " << page;
//        if (QFileInfo(page).exists())
//            infoWindow->showInfo(page);

        runScript();
    }
}


void MainWindow::loadAddOn()
{
    QString currentModel = modelComboBox->currentText();
    if (currentModel.isEmpty())
        return; // for now we don't want users attempting load add-ons without models

    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load add-on"),
                                                    trialsDir,
                                                    "Add-ons (" + currentModel + ".*.eNa)");
    if (!fileName.isEmpty())
    {
        // load and run script
        loadFile(fileName);

//        // load JSON file, if it exists
//        QFileInfo fi(fileName);
//        QString base = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName());

//        // set up signal - slots so that loadLayout will be called
//        // once all of the layers/connections have descriptions
//        designWindow->prepareToLoadLayout(base);
//        conversionWindow->prepareToLoadLayout(base);

//        // show info page, if there is one
//        QString page = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName());
//        page.append(".html");
//        qDebug() << "page = " << page;
//        if (QFileInfo(page).exists())
//            infoWindow->showInfo(page);

        runScript();
    }
}

void MainWindow::loadStimulusSet()
{
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load stimulus set"),
                                                    stimDir,
                                                    tr("Database Files (*.eNd);;Text files (*.txt *.csv);;All files (*.*)"));
    fileName = QDir(easyNetDataHome).relativeFilePath(fileName);
    if (!fileName.isEmpty())
    {
        // create db
        QFileInfo fi(fileName);
        QString base = fi.baseName();

 /*
  *        SessionManager::instance()->runCmd(QStringList({
                                        QString("create stimulus_set %1").arg(base),
                                        QString("%1 load %2").arg(base).arg(fileName),
                                                       }));
                                        //                                         ,
                                        //               QString("xml %1 get").arg(base)
*/

        LazyNutJob *job = new LazyNutJob;
        job->cmdList = QStringList({
               QString("create stimulus_set %1").arg(base),
               QString("%1 load %2").arg(base).arg(fileName),
               QString("xml %1 get").arg(base)
                                     });
        job->setAnswerReceiver(stimSetForm, SLOT(addDataFrameToWidget(QDomDocument*)), AnswerFormatterType::XML);
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job
                << SessionManager::instance()->updateObjectCatalogueJobs();

        SessionManager::instance()->submitJobs(jobs);

        // change combobox text
        stimSetForm->setTableText(base);

        //show Stimuli;
        methodsDock->raise();
        methodsPanel->setCurrentIndex(stimSetTabIdx); // show StimSet tab

    }
}

void MainWindow::importDataFrame()
{
    // bring up file dialog
    QString base;
    QString fileName = QFileDialog::getOpenFileName(this,tr("Import dataframe"),
                                                    stimDir,
                                                    tr("Database Files (*.eNd);;Text files (*.txt);;All files (*.*)"));
    fileName = QDir(easyNetDataHome).relativeFilePath(fileName);
    if (!fileName.isEmpty())
    {
        // create db
        QFileInfo fi(fileName);
        base = fi.baseName();
        df_name_for_updating_combobox = base;
        connect(SessionManager::instance(),SIGNAL(commandsCompleted()),
                                                  this,SLOT(updateDFComboBox()));

        SessionManager::instance()->runCmd(QStringList({
                                         QString("create dataframe %1").arg(base),
                                         QString("%1 load %2").arg(base).arg(fileName)}));

    }
}

void MainWindow::updateDFComboBox()
{
    //show new dataframe;
    explorerDock->raise();
    explorerPanel->setCurrentIndex(dfTabIdx);
    dataframesWindow->selectTable(df_name_for_updating_combobox);
    disconnect(SessionManager::instance(),SIGNAL(commandsCompleted()),
                                              this,SLOT(updateDFComboBox()));

}



void MainWindow::currentStimulusChanged(QString stim)
{
//    if (inputComboBox->findText(stim) == -1)
//        inputComboBox->addItem(stim);
//    inputComboBox->setCurrentIndex(inputComboBox->findText(stim));


}

void MainWindow::loadFile(const QString &fileName)
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

    //statusBar()->showMessage(tr("File loaded"), 2000);
}

void MainWindow::readSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    if(qEnvironmentVariableIsSet("EASYNET_HOME"))
    {
       easyNetHome=QDir::fromNativeSeparators(QString::fromUtf8(qgetenv("EASYNET_HOME")));
    }
    else
    {
       easyNetHome = settings.value("easyNetHome","../..").toString();
    }
    if(qEnvironmentVariableIsSet("EASYNET_DATA_HOME"))
    {
       easyNetDataHome=QDir::fromNativeSeparators(QString::fromUtf8(qgetenv("EASYNET_DATA_HOME")));
    }
    else
    {
       easyNetDataHome = settings.value("easyNetDataHome",easyNetHome).toString();
    }
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();

//    resize(size);
    move(pos);

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();
    writeSettings(); // in case some setting was missing. Settings are read by the GUI,
    // e.g. easyNetHome, since it's private to EasyNetMainWindow.
}

void MainWindow::setDefaultLocations()
{
    lazyNutBat = QString("%1/%2/nm_files/%3").arg(easyNetHome).arg(binDir).arg(lazyNutBasename);
    scriptsDir = QString("%1/Models").arg(easyNetDataHome);
    trialsDir = QString("%1/Trials").arg(easyNetDataHome);
    stimDir = QString("%1/Databases/Stimulus_files").arg(easyNetDataHome);
}

void MainWindow::writeSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    settings.setValue("easyNetHome", easyNetHome);
    settings.setValue("easyNetDataHome", easyNetDataHome);
    settings.setValue("pos", pos());
    settings.setValue("size", size());

    settings.beginGroup("mainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();
}

bool MainWindow::proceedWithRestart()
{
    QMessageBox::StandardButton reply =
            QMessageBox::warning(
                this,
                "Restart interpreter",
                "The operation you have chosen requires to restart the interpreter."
                "This will start a new session,  overwiting all data associated with the current session. Do you want to proceed?",
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return false;

    return true;
}



void MainWindow::closeEvent(QCloseEvent *event)
{
        writeSettings();
        SessionManager::instance()->killLazyNut();
        qDebug() << "eNMainWindow emitting saveLayout";
        emit saveLayout();
        event->accept();
}

void MainWindow::checkScreens()
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

void MainWindow::setCurrentFile(EditWindow *window, const QString &fileName)
{
    window->setCurrentFile(fileName);
    window->textEdit->document()->setModified(false);
    setWindowModified(false);

//    curJson = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName().append(".json"));
}

void MainWindow::newScriptFile()
{
    newFile(scriptEdit);
}

void MainWindow::newLogFile()
{
    newFile(commandLog);
}

void MainWindow::newFile(EditWindow* window)
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

void MainWindow::loadScript()
{
 //   if (maybeSave())
//    {
        QString fileName = QFileDialog::getOpenFileName(this,tr("Open script"), scriptsDir, tr("Script Files (*.eNs *.eNm)"));
        if (!fileName.isEmpty())
        {
            loadFile(fileName);
            lazynutPanel->setCurrentIndex(scriptTabIdx); // show scripts tab
            codePanelDock->show();
        }
 //   }
}




//! [runCmdAndUpdate]
void MainWindow::runCmdAndUpdate(QStringList cmdList)
{
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList = cmdList;
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->updateObjectCatalogueJobs();

    SessionManager::instance()->submitJobs(jobs);
}
//! [runCmdAndUpdate]

//! [getVersion]
void MainWindow::getVersion()
{
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({"version"});
    job->setAnswerReceiver(this, SLOT(displayVersion(QString)), AnswerFormatterType::Identity);
    SessionManager::instance()->submitJobs(job);
}

//void EasyNetMainWindow::showDocumentation()
//{
//    assistant->showDocumentation("index.html");
//}

void MainWindow::viewSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    QStringList keys = settings.allKeys();
    QString details = keys.join("\n");

    QVector <QLineEdit*> editList;

    QGroupBox *groupBox = new QGroupBox(tr("Settings"));
    QFormLayout *formLayout = new QFormLayout;
    foreach (QString key, keys)
    {
        editList.push_back(new QLineEdit(settings.value(key).toString()));
        formLayout->addRow(key, editList.back());
    }
    groupBox->setLayout(formLayout);
    groupBox->show();
}

void MainWindow::setEasyNetHome()
{
    if (!proceedWithRestart())
        return;
    QSettings settings("QtEasyNet", "nmConsole");
    easyNetHome = QFileDialog::getExistingDirectory(
                this,
                tr("New easyNet home directory"),
                settings.value("easyNetHome","../..").toString()
                );
    qDebug() << "easyNetHome "<< easyNetHome;
    if (easyNetHome.isEmpty())
        return;

    settings.setValue("easyNetHome", easyNetHome);
    setDefaultLocations();
    SessionManager::instance()->restartLazyNut(lazyNutBat);
}

void MainWindow::setEasyNetDataHome()
{
    if (!proceedWithRestart())
        return;
    QSettings settings("QtEasyNet", "nmConsole");
    easyNetDataHome = QFileDialog::getExistingDirectory(
                this,
                tr("New easyNet data home directory"),
                settings.value("easyNetDataHome",easyNetHome).toString());
    qDebug() << "easyNetDataHome" << easyNetDataHome;

    if (easyNetDataHome.isEmpty())
        return;

    settings.setValue("easyNetDataHome", easyNetDataHome);
    setDefaultLocations();
    SessionManager::instance()->restartLazyNut(lazyNutBat);
}


void MainWindow::setFontSize(const QString & size)
{
    int fontSize;
    if (size == "small")
        fontSize = EN_FONT_SMALL;
    else if (size == "medium")
         fontSize = EN_FONT_MEDIUM;
    else if (size == "large")
         fontSize = EN_FONT_LARGE;
    else
        fontSize = EN_FONT_SMALL;

    QApplication::setFont(QFont(EN_FONT, fontSize));
    lazyNutConsole2->setConsoleFontSize(fontSize);

}

//void EasyNetMainWindow::showPauseState(bool isPaused)
//{
//    if (isPaused)
//        pauseAct->setIconText("RESUME");
//    else
//        pauseAct->setIconText("PAUSE");
//}


void MainWindow::lazyNutNotRunning()
{
    QMessageBox::critical(this, "critical",
    QString("%1 script not running or not found.\n"
            "Please select a valid %1 file from the menu Settings -> Set %1\n"
            "or a valid easyNet home directory using the menu Settings -> Set easyNet home directory").arg(lazyNutBasename));
}

void MainWindow::displayVersion(QString version)
{
    QMessageBox::about(this, "Version",QString("LazyNut version is:\n%1").arg(version));
}


void MainWindow::createActions()
{
//    createViewActions();

//    runAction = new QAction(QIcon(":/images/media-play-8x.png"),tr("&Run"), this);
//    runAction->setStatusTip(tr("Run"));
//    connect(runAction,SIGNAL(triggered()),this, SLOT(run()));

    newScriptAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newScriptAct->setShortcuts(QKeySequence::New);
//    newScriptAct->setStatusTip(tr("Create a new file"));
    connect(newScriptAct, SIGNAL(triggered()), this, SLOT(newScriptFile()));

    newLogAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newLogAct->setShortcuts(QKeySequence::New);
//    newLogAct->setStatusTip(tr("Create a new file"));
    connect(newLogAct, SIGNAL(triggered()), this, SLOT(newLogFile()));

    loadModelAct = new QAction(QIcon(":/images/layers-2x.png"), tr("&Load model"), this);
//    loadModelAct->setShortcuts(QKeySequence::Open);
    loadModelAct->setStatusTip(tr("Load a previously specified model"));
    connect(loadModelAct, SIGNAL(triggered()), this, SLOT(loadModel()));

    loadTrialAct = new QAction(QIcon(":/images/cog-2x.png"), tr("&Load trial"), this);
//    loadTrialAct->setShortcuts(QKeySequence::Open);
    loadTrialAct->setStatusTip(tr("Load a previously specified trial"));
    connect(loadTrialAct, SIGNAL(triggered()), this, SLOT(loadTrial()));

    loadAddOnAct = new QAction(QIcon(":/images/add-on.png"), tr("&Load add-on"), this);
//    loadAddOnAct->setShortcuts(QKeySequence::Open);
    loadAddOnAct->setStatusTip(tr("Load an add-on to extend the model"));
    connect(loadAddOnAct, SIGNAL(triggered()), this, SLOT(loadAddOn()));
    loadAddOnAct->setEnabled(false);

    importDataFrameAct = new QAction(QIcon(":/images/list-2x.png"), tr("&Import dataframe"), this);
//    importDataFrameAct->setShortcuts(QKeySequence::Open);
    importDataFrameAct->setStatusTip(tr("Import a database/dataframe"));
    connect(importDataFrameAct, SIGNAL(triggered()), this, SLOT(importDataFrame()));

    loadStimulusSetAct = new QAction(QIcon(":/images/open.png"), tr("Load &stimulus set"), this);
    loadStimulusSetAct->setStatusTip(tr("Load a stimulus set"));
    connect(loadStimulusSetAct, SIGNAL(triggered()), this, SLOT(loadStimulusSet()));

    loadScriptAct = new QAction(QIcon(":/images/code-2x.png"), tr("&Open a script"), this);
//    loadScriptAct->setShortcuts(QKeySequence::Open);
    loadScriptAct->setStatusTip(tr("Open an existing lazyNut script"));
    connect(loadScriptAct, SIGNAL(triggered()), this, SLOT(loadScript()));
    loadScriptAct->setEnabled(true); // no script loading until after UKOG!

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    restartInterpreterAct = new QAction(tr("Restart Interpreter"), this);
    restartInterpreterAct->setStatusTip(tr("Restart the lazyNut interpreter, to start a new session"));
    connect(restartInterpreterAct, SIGNAL(triggered()), this, SLOT(restart()));
    restartInterpreterAct->setDisabled(true);


    viewSettingsAct = new QAction(tr("View easyNet settings"), this);
    viewSettingsAct->setStatusTip(tr("View easyNet settings"));
    connect(viewSettingsAct,SIGNAL(triggered()),this, SLOT(viewSettings()));

    setEasyNetHomeAct = new QAction(tr("Set easyNet home directory"), this);
    setEasyNetHomeAct->setStatusTip(tr("Set easyNet home directory"));
    connect(setEasyNetHomeAct,SIGNAL(triggered()),this, SLOT(setEasyNetHome()));

    setEasyNetDataHomeAct = new QAction(tr("Set easyNet data home directory"), this);
    setEasyNetDataHomeAct->setStatusTip(tr("Set easyNet data home directory"));
    connect(setEasyNetDataHomeAct,SIGNAL(triggered()),this, SLOT(setEasyNetDataHome()));



    setFontSignalMapper = new QSignalMapper(this);
    setFontActGrouop = new QActionGroup(this);

    setSmallFontAct = new QAction(QString(tr("Small")), this);
    setSmallFontAct->setStatusTip(QString(tr("Switch to a smaller font")));
    setSmallFontAct->setCheckable(true);
    setFontSignalMapper->setMapping(setSmallFontAct, "small");
    connect(setSmallFontAct,SIGNAL(triggered()), setFontSignalMapper, SLOT(map()));
    setFontActGrouop->addAction(setSmallFontAct);

    setMediumFontAct = new QAction(QString(tr("Medium")), this);
    setMediumFontAct->setStatusTip(QString(tr("Switch to a medium font")));
    setMediumFontAct->setCheckable(true);
    setFontSignalMapper->setMapping(setMediumFontAct, "medium");
    connect(setMediumFontAct,SIGNAL(triggered()), setFontSignalMapper, SLOT(map()));
    setFontActGrouop->addAction(setMediumFontAct);

    setLargeFontAct = new QAction(QString(tr("Large")), this);
    setLargeFontAct->setStatusTip(QString(tr("Switch to a larger font")));
    setLargeFontAct->setCheckable(true);
    setFontSignalMapper->setMapping(setLargeFontAct, "large");
    connect(setLargeFontAct,SIGNAL(triggered()), setFontSignalMapper, SLOT(map()));
    setFontActGrouop->addAction(setLargeFontAct);

    connect(setFontSignalMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(setFontSize(const QString &)));

    versionAct = new QAction("Version",this);
    connect(versionAct,SIGNAL(triggered()),this,SLOT(getVersion()));

    assistantAct = new QAction(tr("Help Contents"), this);
    assistantAct->setShortcut(QKeySequence::HelpContents);
    connect(assistantAct, SIGNAL(triggered()), this, SLOT(showDocumentation()));

    setQuietModeAct = new QAction(tr("Quiet mode"), this);
    setQuietModeAct->setCheckable(true);
    connect(setQuietModeAct, SIGNAL(triggered()), this, SLOT(setQuietMode()));
    setQuietModeAct->setChecked(true);
}

void MainWindow::restart()
{
    if (!proceedWithRestart())
        return;

    SessionManager::instance()->restartLazyNut(lazyNutBat);
}

void MainWindow::showDocumentation()
{
    assistant->showDocumentation("index.html");
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));

    fileSubMenu = fileMenu->addMenu(tr("&Load"));
    fileSubMenu->addAction(loadModelAct);
    fileSubMenu->addAction(loadTrialAct);
    fileSubMenu->addAction(loadAddOnAct);
    fileSubMenu->addAction(loadStimulusSetAct);
    fileSubMenu->addAction(loadScriptAct);
    fileSubMenu->addAction(importDataFrameAct);

    //fileMenu->addAction(newAct);
    //fileMenu->addAction(openAct);
    //fileMenu->addAction(saveAct);
    //fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(restartInterpreterAct);
    fileMenu->addAction(exitAct);

    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    settingsMenu->addAction(viewSettingsAct);
    settingsMenu->addAction(setEasyNetHomeAct);
    settingsMenu->addAction(setEasyNetDataHomeAct);
    settingsSubMenu = settingsMenu->addMenu(tr("&Font size"));
    settingsSubMenu->addAction(setSmallFontAct);
    settingsSubMenu->addAction(setMediumFontAct);
    settingsSubMenu->addAction(setLargeFontAct);
    settingsMenu->addSeparator();
    settingsMenu->addAction(setQuietModeAct);

//    settingsMenu->addAction(synchModeAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    menuBar()->addSeparator();

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(assistantAct);
//    helpMenu->addSeparator();
//    helpMenu->addAction(versionAct);

    menuBar()->addMenu(helpMenu);
}


/******** StatusBar code *********/

void MainWindow::createStatusBar()
{
    lazyNutProgressBar = new QProgressBar;
    lazyNutProgressBar->setTextVisible(false);
    lazyNutProgressBar->setMinimum(0);
    statusBar()->addPermanentWidget(lazyNutProgressBar, 0.5);
    connect(SessionManager::instance(), SIGNAL(commandsInJob(int)),
            lazyNutProgressBar, SLOT(setMaximum(int)));
    connect(SessionManager::instance(), SIGNAL(commandExecuted(QString,QString)),
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
    statusBar()->addPermanentWidget(lazyNutStatusWidget, 0);
    connect(SessionManager::instance(), SIGNAL(isReady(bool)), this, SLOT(setLazyNutIsReady(bool)));
    connect(SessionManager::instance(), &SessionManager::lazyNutNotRunning,[=](){
        lazyNutStatusWidget->setCurrentWidget(offLabel);
    });




    lazyNutCmdLabel = new QLabel;
    lazyNutCmdLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    lazyNutCmdLabel->setFixedWidth(qApp->fontMetrics().width("LAST COMMAND: a very very very very long lazyNut command"));
    lazyNutCmdLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    statusBar()->addWidget(lazyNutCmdLabel, 1);
    connect(SessionManager::instance(), SIGNAL(commandExecuted(QString,QString)),
            this, SLOT(showCmdOnStatusBar(QString)));

//     connect(SessionManager::instance(), &SessionManager::commandExecuted, [=](QString cmd, QString /*timString*/)
//     {
//        statusBar()->showMessage(QString("LAST EXEC COMMAND: %1").arg(cmd));
//     });
//    lazyNutErrorLabel = new QLabel;
//    lazyNutErrorLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
//    lazyNutErrorLabel->setStyleSheet("QLabel {"
//                                 "font-weight: bold;"
//                                 "color: red"
//                                 "}");
//    statusBar()->addWidget(lazyNutErrorLabel, 1);

//    lazyNutErrorBox = new QComboBox;
//    lazyNutErrorBox->setToolTip("list of lazyNut ERRORs");
//    lazyNutErrorBox->addItem("");
//    lazyNutErrorBox->setStyleSheet("QComboBox {"
//                                 "font-weight: bold;"
//                                 "color: red"
//                                 "}");
//    lazyNutErrorBox->setEditable(false);
//    statusBar()->addWidget(lazyNutErrorBox, 1);
//    connect(lazyNutErrorBox,SIGNAL(activated(int)),this,SLOT(showMostRecentError()));

    connect(SessionManager::instance(), &SessionManager::cmdError, [=](QString /*cmd*/, QStringList errorList)
    {
       if (!errorList.isEmpty())
           statusBar()->showMessage(errorList.last(), 4000);
    });
    connect(statusBar(), &QStatusBar::messageChanged, [=](QString msg)
    {
       if (msg.startsWith("ERROR"))
           statusBar()->setStyleSheet("color: red");
       else
           statusBar()->setStyleSheet("color: black");
    });


//    connect(SessionManager::instance(), SIGNAL(cmdError(QString,QStringList)),
//            this, SLOT(showErrorOnStatusBar(QString,QStringList)));
//    connect(SessionManager::instance(), SIGNAL(lazyNutMacroStarted()),
//            this, SLOT(clearErrorOnStatusBar()));
}

void MainWindow::showMostRecentError()
{
    lazyNutErrorBox->setCurrentIndex(lazyNutErrorBox->count()-1);
}

void MainWindow::setLazyNutIsReady(bool isReady)
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

void MainWindow::showErrorOnStatusBar(QString /*cmd*/, QStringList errorList)
{
//    lazyNutErrorBox->addItems(errorList.replaceInStrings(QRegExp("^(.*)$"), "LAST ERROR: \\1"));
    lazyNutErrorBox->addItems(errorList);
    lazyNutErrorBox->setCurrentIndex(lazyNutErrorBox->count() - 1);
}

void MainWindow::clearErrorOnStatusBar()
{
//    lazyNutErrorLabel->clear();
    lazyNutErrorBox->setCurrentText("");
}

void MainWindow::showCmdOnStatusBar(QString cmd)
{
    lazyNutCmdLabel->setText(QString("LAST COMMAND: %1").arg(cmd));
}

void MainWindow::addOneToLazyNutProgressBar()
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


void MainWindow::processHistoryKey(int dir)
{
    qDebug() << "processHistoryKey" << dir;
    QString line = commandLog->getHistory(dir);
    qDebug() << "processHistoryKey string = " << line;
    emit showHistory(line);
}
