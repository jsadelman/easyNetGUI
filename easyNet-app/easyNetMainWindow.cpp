#include <QtWidgets>
#include <QtGlobal>
#include <QtDebug>
#include <QTreeView>
#include <QStringList>
#include <QEventLoop>
#include <QToolBar>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QtCore/QLibraryInfo>
#include <QMessageBox>
#include <QDir>
#include <QFontMetrics>
#include <QFont>
#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>


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
//#include "plotsettingswindow.h"
#include "lazynutjobparam.h"
#include "lazynutjob.h"
#include "objectcache.h"
#include "objectcachefilter.h"
//#include "lazynutscripteditor.h"
#include "maxminpanel.h"
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
#include "plotviewerdispatcher.h"
#include "diagramscenetabwidget.h"
#include "diagramscenestackedwidget.h"
#include "diagramscene.h"
#include "diagramwindow.h"
#include "enumclasses.h"
#include "tablewindow.h"
#include "dataframeviewer.h"
#include "dataframeviewerdispatcher.h"
#include "ui_datatabsviewer.h"
#include "ui_datacomboviewer.h"
#include "settingsform.h"
#include "settingsformdialog.h"
#include "modelsettingsdisplay.h"
#include "settingswidget.h"
#include "floatingDialogWindow.h"


MainWindow* MainWindow::mainWindow = nullptr;

MainWindow* MainWindow::instance()
{
    return mainWindow ? mainWindow : (mainWindow = new MainWindow);
}

void MainWindow::build()
{
    readSettings();
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

    /* INITIAL DISPLAY AT STARTUP */
    //diagramDock->raise();
    diagramPanel->setCurrentIndex(modelTabIdx);
    diagramSceneTabChanged(modelTabIdx);
    setQuietMode(true);

    #ifdef WIN32
    if (qApp->arguments().count() > 1)
        loadModel(QDir::fromNativeSeparators(qApp->arguments().at(1)),true);
    #endif

    SessionManager::instance()->startLazyNut();


}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_debugMode(false), m_trialListLength(0)
{
}


void MainWindow::constructForms()
{
expertWindow=new QMainWindow;

    /* CONSTRUCT TABWIDGETS */
/*
    introPanel = new QTabWidget;
    introPanel->setMovable(true);
    introPanel->setTabsClosable(true);
*/

    lazynutPanel = new QTabWidget;
    methodsPanel = new QTabWidget;
    diagramPanel = new DiagramSceneStackedWidget(this);

    lazynutPanel->setMovable(true);
    diagramWindow = new DiagramWindow(diagramPanel, this);
    methodsPanel->addTab(diagramWindow,"Model");

    /* CONSTRUCT ALL THE INDIVIDUAL FORMS */

    lazyNutConsole = new Console(this);

    objExplorer = new ObjExplorer(SessionManager::instance()->descriptionCache,this);
    scriptEdit = new ScriptEditor(SessionManager::instance()->defaultLocation("scriptsDir"), this);
    highlighter = new Highlighter(scriptEdit->textEdit->document());
//    commandLog = new EditWindow(this, newLogAct, loadScriptAct, true); // no cut, no paste
    commandLog = new CommandLog(this, false);
    highlighter2 = new Highlighter(commandLog->textEdit->document());
    errorLog = new CommandLog(this, true);
    highlighter3 = new Highlighter(errorLog->textEdit->document());
    rLog = new CommandLog(this, true);

    debugLog = new DebugLog (this);
//    welcomeScreen = new QWebView(this);
//    welcomeScreen->setUrl(QUrl("qrc:///images/Welcome.html"));
//    stimSetForm = new TableEditor ("Stimuli",this);
    ui_stimSetViewer = new Ui_DataTabsViewer;
    stimSetViewer = new DataframeViewer(ui_stimSetViewer, this);
    stimSetViewer->setDragDropColumns(true);
    stimSetViewer->setStimulusSet(true);
    stimSetViewer->setDefaultDir(SessionManager::instance()->defaultLocation("stimDir"));

//    tablesWindow = new TableEditor (SessionManager::instance()->descriptionCache,"Tables",this);
//    tableWindow = new TableViewer("Tables",this);
//    tableWindow = new TableWindow(this);

    ui_dataframeResultsViewer = new Ui_DataTabsViewer;
    ui_dataframeResultsViewer->setUsePrettyNames(true);
    dataframeResultsViewer = new DataframeViewer(ui_dataframeResultsViewer, this);
    dataframeResultsDispatcher = new DataframeViewerDispatcher(dataframeResultsViewer);
    dataframeResultsViewer->setDefaultDir(SessionManager::instance()->defaultLocation("dfDir"));

    ui_dataframeViewer = new Ui_DataComboViewer;
    dataframeViewer = new DataframeViewer(ui_dataframeViewer, this);
    dataframeViewer->setLazy(true);
    dataframeViewer->setDefaultDir(SessionManager::instance()->defaultLocation("dfDir"));
    dataframeDescriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    dataframeDescriptionFilter->setType("dataframe");
    connect(dataframeDescriptionFilter, SIGNAL(objectCreated(QString,QString,QString,QDomDocument*)),
            dataframeViewer, SLOT(addItem(QString)));


//    dataframesWindow = new TableEditor(SessionManager::instance()->descriptionCache,"Dataframes",this);


//    paramEdit = new TableEditor ("Parameters",this);
    ui_paramViewer = new Ui_DataTabsViewer;
    paramViewer = new DataframeViewer(ui_paramViewer, this);
    paramViewer->setParametersTable(true);
    paramDescriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    paramDescriptionFilter->setFilterRegExp(QRegExp("^\\(.* parameters\\)$"));
    paramDescriptionFilter->setFilterKeyColumn(ObjectCache::NameCol);
    connect(paramDescriptionFilter, SIGNAL(objectCreated(QString,QString,QString,QDomDocument*)),
            paramViewer, SLOT(addItem(QString)));


    plotViewer = new PlotViewer(ui_dataframeResultsViewer , this);
    plotViewerDispatcher = new PlotViewerDispatcher(plotViewer);
    plotDescriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    plotDescriptionFilter->setType("xfile");
    connect(plotDescriptionFilter, SIGNAL(objectCreated(QString,QString,QString,QDomDocument*)),
            plotViewer, SLOT(addItem(QString)));

    ui_testViewer = new Ui_DataTabsViewer;
    testViewer = new DataframeViewer(ui_testViewer, this);
    testFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    testFilter->setName("test_results");
    testFilter->setFilterKeyColumn(ObjectCache::NameCol);
    connect(testFilter, SIGNAL(objectCreated(QString,QString,QString,QDomDocument*)),
            testViewer, SLOT(addItem(QString)));

    trialEditor = new TrialEditor(this);

    modelSettingsDisplay = new ModelSettingsDisplay(this);
    modelSettingsDialog = new FloatingDialogWindow(this);
    modelSettingsDialog->setCentralWidget(modelSettingsDisplay);
    modelSettingsDisplay->setCommand("list_settings");
//    modelSettingsDisplay->hide();


    paramSettingsDialog = new FloatingDialogWindow(this);
    paramSettingsDialog->setCentralWidget(paramViewer);
//    paramSettingsDialog->hide();

    dataViewSettingsWidget = new SettingsWidget(this);
    dataViewSettingsDialog = new FloatingDialogWindow(this);
    dataViewSettingsDialog->setCentralWidget(dataViewSettingsWidget);


    /* ADD TABS */
//    infoTabIdx = introPanel->addTab(infoWindow, tr("Intro"));

    modelTabIdx = diagramPanel->newDiagramScene(tr("Model"), "layer", "connection");
    modelScene = diagramPanel->diagramSceneAt(modelTabIdx);
//    diagramPanel->hide();
    diagramPanel->show();

    //    conversionTabIdx = diagramPanel->newDiagramScene(tr("Conversions"), "representation", "conversion");
//    conversionScene = diagramPanel->diagramSceneAt(conversionTabIdx);
//    conversionPage = diagramPanel->widget(conversionTabIdx);

    stimSetTabIdx = methodsPanel->addTab(stimSetViewer, tr("Stimuli"));
    trialFormTabIdx = methodsPanel->addTab(trialEditor, tr("Trial")); //textEdit1
//    modelSettingsTabIdx = methodsPanel->addTab(modelSettingsDisplay, tr("Model"));
//    paramTabIdx = methodsPanel->addTab(paramViewer, tr("Parameters"));
//    plotSettingsTabIdx = methodsPanel->addTab(plotSettingsWindow, tr("Plot settings"));
//    dataViewSettingsTabIdx = methodsPanel->addTab(dataViewSettingsWidget, tr("Dataframe/plot settings"));

    lazynutPanel->addTab(lazyNutConsole, tr("Console"));
    lazynutPanel->addTab(commandLog, tr("History"));
    lazynutPanel->addTab(errorLog, tr("Errors"));
    lazynutPanel->addTab(rLog, tr("R"));
    testsTabIdx = lazynutPanel->addTab(testViewer, tr("Tests"));
    scriptTabIdx = lazynutPanel->addTab(scriptEdit, tr("Script"));
    lazynutPanel->addTab(debugLog, tr("Debug log"));

//    infoTabIdx = explorerPanel->addTab(infoWindow, tr("Info"));
    explorerTabIdx = lazynutPanel->addTab(objExplorer, tr("Explorer"));
//    dfTabIdx = explorerPanel->addTab(dataframesWindow, tr("Dataframes"));
    dfTabIdx = lazynutPanel->addTab(dataframeViewer, tr("All Dataframes"));

//   outputTablesTabIdx= plotTabIdx = resultsPanel->addTab(plotViewer, tr("Plots"));
//    outputTablesTabIdx = resultsPanel->addTab(tableWindow, tr("Tables"));
//    plotTabIdx=outputTablesTabIdx = resultsPanel->addTab(dataframeResultsViewer, tr("Results"));


    // perhaps use this code for detachable tabs?
    // http://www.qtcentre.org/threads/61403-SOLVED-Detachable-QDockWidget-tabs
}


void MainWindow::connectSignalsAndSlots()
{
    connect(modelComboBox, SIGNAL(currentIndexChanged(QString)), SessionManager::instance(), SLOT(setCurrentModel(QString)));

    // synch between DataViewers and dataViewSettingsWidget
    connect(plotViewer,SIGNAL(showSettingsRequested()),this,SLOT(showDataViewSettings()));
    connect(plotViewer, &PlotViewer::showSettingsRequested, [=]()
    {
       setFormAndShow(plotViewer->currentItemName());
    });
    connect(dataframeResultsViewer, SIGNAL(showSettingsRequested()), this,SLOT(showDataViewSettings()));
    connect(dataframeViewer, SIGNAL(showSettingsRequested()), this,SLOT(showDataViewSettings()));
    connect(stimSetViewer, SIGNAL(showSettingsRequested()), this,SLOT(showDataViewSettings()));

    connect(plotViewer,SIGNAL(currentItemChanged(QString)), this, SLOT(setFormInSettingsWidget(QString)));
    connect(dataframeResultsViewer, SIGNAL(currentItemChanged(QString)), this, SLOT(setFormInSettingsWidget(QString)));
    connect(dataframeViewer, SIGNAL(currentItemChanged(QString)), this, SLOT(setFormInSettingsWidget(QString)));
    connect(stimSetViewer, SIGNAL(currentItemChanged(QString)), this, SLOT(setFormInSettingsWidget(QString)));

//    connect(resultsPanel, SIGNAL(currentChanged(int)), this, SLOT(switchFormInSettingsWidget()));
 //   connect(explorerPanel, SIGNAL(currentChanged(int)), this, SLOT(switchFormInSettingsWidget()));
//    connect(methodsPanel, SIGNAL(currentChanged(int)), this, SLOT(switchFormInSettingsWidget()));

    connect(resultsDock, SIGNAL(visibilityChanged(bool)), this, SLOT(switchFormInSettingsWidget(bool)));
//   connect(explorerDock, SIGNAL(visibilityChanged(bool)), this, SLOT(switchFormInSettingsWidget(bool)));
//    connect(methodsDock, SIGNAL(visibilityChanged(bool)), this, SLOT(switchFormInSettingsWidget(bool)));
    // cannot apply this to methodsPanel/Dock because methodsPanel sits in the same Dock as dataViewSettingsWidget


    //
    connect(diagramPanel, SIGNAL(currentChanged(int)), this, SLOT(diagramSceneTabChanged(int)));
    connect(scriptEdit,SIGNAL(runCmdRequested(QStringList)),SessionManager::instance(),SLOT(runCmd(QStringList)));
    connect(SessionManager::instance(),SIGNAL(userLazyNutOutputReady(QString)),
            lazyNutConsole,SLOT(addText(QString)));
    connect(lazyNutConsole,SIGNAL(historyKey(int, QString)),
            this,SLOT(processHistoryKey(int, QString)));
    connect(this,SIGNAL(showHistory(QString)),
            lazyNutConsole,SLOT(showHistory(QString)));
//    connect(trialComboBox,SIGNAL(currentIndexChanged(QString)),
//            trialEditor,SLOT(setTrialName(QString)));
    connect(trialComboBox,SIGNAL(currentIndexChanged(QString)),
            SessionManager::instance(), SLOT(setCurrentTrial(QString)));
    connect(modelScene,SIGNAL(objectSelected(QString)), objExplorer,SIGNAL(objectSelected(QString)));
    connect(modelScene,SIGNAL(objectSelected(QString)), this,SLOT(showExplorer()));
    connect(modelScene,SIGNAL(createDataViewRequested(QString,QString,QString,QMap<QString,QString>, bool)),
            dataViewSettingsWidget,SLOT(newForm(QString,QString,QString,QMap<QString,QString>, bool)));
//    connect(plotViewer,SIGNAL(createNewRPlot(QString,QString,QMap<QString,QString>, bool)),
//            dataViewSettingsWidget,SLOT(newForm(QString,QString,QMap<QString,QString>, bool)));
    connect(trialWidget, SIGNAL(aboutToRunTrial(QSharedPointer<QDomDocument> )),
            dataframeResultsViewer, SLOT(preDispatch(QSharedPointer<QDomDocument> )));
    connect(trialWidget, SIGNAL(aboutToRunTrial(QSharedPointer<QDomDocument> )),
            plotViewer, SLOT(preDispatch(QSharedPointer<QDomDocument> )));
//     connect(plotSettingsWindow, SIGNAL(newRPlotCreated(QString, bool, bool, QList<QSharedPointer<QDomDocument> >)),
//             plotViewer, SLOT(addItem(QString, bool, bool, QList<QSharedPointer<QDomDocument> >)));
     connect(dataViewSettingsWidget, SIGNAL(dataViewCreated(QString,bool)),
             dataframeViewer, SLOT(addRequestedItem(QString,bool)));
     connect(dataViewSettingsWidget, SIGNAL(dataViewCreated(QString,bool)),
             dataframeResultsViewer, SLOT(addRequestedItem(QString,bool)));
     connect(dataViewSettingsWidget, SIGNAL(dataViewCreated(QString,bool)),
             stimSetViewer, SLOT(addRequestedItem(QString,bool)));
     connect(dataViewSettingsWidget, SIGNAL(dataViewCreated(QString,bool)),
             plotViewer, SLOT(addRequestedItem(QString,bool)));
     connect(plotViewer, SIGNAL(addDataframeRequested(QString,bool)), dataframeResultsViewer, SLOT(addItem(QString,bool)));

     connect(trialWidget, SIGNAL(trialRunModeChanged(int)), dataframeResultsViewer, SLOT(setTrialRunMode(int)));
     connect(trialWidget, SIGNAL(trialRunModeChanged(int)), plotViewer, SLOT(setTrialRunMode(int)));

    connect(dataframeResultsViewer, SIGNAL(itemRemoved(QString)), dataViewSettingsWidget, SLOT(removeForm(QString)));
    connect(stimSetViewer, SIGNAL(itemRemoved(QString)), dataViewSettingsWidget, SLOT(removeForm(QString)));
    connect(dataframeViewer, SIGNAL(itemRemoved(QString)), dataViewSettingsWidget, SLOT(removeForm(QString)));
    connect(plotViewer, SIGNAL(itemRemoved(QString)), dataViewSettingsWidget, SLOT(removeForm(QString)));
    connect(diagramWindow,SIGNAL(showModelSettingsSignal()), this,SLOT(showModelSettings()));
    connect(diagramWindow,SIGNAL(showParameterSettingsSignal()), this,SLOT(showParameterSettings()));
    connect(diagramWindow, SIGNAL(loadModelSignal()), this, SLOT(loadModel()));
    connect(trialEditor, SIGNAL(loadTrialSignal()), this, SLOT(loadTrial()));


    connect(dataframeResultsViewer, SIGNAL(createDataViewRequested(QString,QString,QString,QMap<QString,QString>,bool)),
            dataViewSettingsWidget, SLOT(newForm(QString,QString,QString,QMap<QString,QString>,bool)));
    connect(stimSetViewer, SIGNAL(createDataViewRequested(QString,QString,QString,QMap<QString,QString>,bool)),
            dataViewSettingsWidget, SLOT(newForm(QString,QString,QString,QMap<QString,QString>,bool)));
    connect(dataframeViewer, SIGNAL(createDataViewRequested(QString,QString,QString,QMap<QString,QString>,bool)),
            dataViewSettingsWidget, SLOT(newForm(QString,QString,QString,QMap<QString,QString>,bool)));

    connect(dataViewSettingsWidget, SIGNAL(settingsApplied(QString)), this, SLOT(showResultsViewer(QString)));

    connect(SessionManager::instance(), SIGNAL(logCommand(QString)),
            commandLog, SLOT(addText(QString)));
    connect(SessionManager::instance(), SIGNAL(commandSent(QString)),
            debugLog, SLOT(addRowToTable(QString)));
    connect(SessionManager::instance(), SIGNAL(commandExecuted(QString,QString)),
            debugLog, SLOT(updateCmd(QString,QString)));
    connect(SessionManager::instance(), SIGNAL(resetExecuted()),
            debugLog, SLOT(skipRemainingCommands()));
//    connect(stopAct, SIGNAL(triggered()), debugLog, SLOT(skipRemainingCommands()));
//    connect(SessionManager::instance(), SIGNAL(cmdError(QString,QStringList)),
//             debugLog, SLOT(skipRemainingCommands()));
    connect(SessionManager::instance(), &SessionManager::cmdError, [=](QString /*cmd*/, QStringList errorList)
    {
       foreach(QString error, errorList)
           errorLog->addText(error);
    });
    connect(SessionManager::instance(), &SessionManager::cmdR, [=](QString /*cmd*/, QStringList rList)
    {
       foreach(QString fb, rList)
           rLog->addText(fb);
    });

    connect(SessionManager::instance(), SIGNAL(dotsCount(int)), this, SLOT(updateTrialRunListCount(int)));
connect(expertShow,SIGNAL(triggered()),this,SLOT(displayExpertWindow()));
}

void MainWindow::showExplorer()
{
/*    explorerDock->show();
    explorerDock->setFocus();
    explorerDock->raise();
    explorerPanel->setCurrentIndex(explorerTabIdx);
*/
  expertWindow->show();
  expertWindow->setFocus();
  expertWindow->raise();
  lazynutPanel->setCurrentIndex(explorerTabIdx);


}

//void MainWindow::showPlotSettings()
//{
//    methodsDock->raise();
//    methodsPanel->setCurrentIndex(plotSettingsTabIdx);
//}

//void MainWindow::showDataViewSettings()
//{
//    methodsDock->raise();
//    methodsPanel->setCurrentIndex(dataViewSettingsTabIdx);
//}

void MainWindow::showPlotViewer()
{
    resultsDock->raise();
//    resultsPanel->setCurrentIndex(plotTabIdx);
}


void MainWindow::diagramSceneTabChanged(int index)
{
     modelScene->goToSleep();
//     conversionScene->goToSleep();
    if (index == modelTabIdx)
         modelScene->wakeUp();
//    else if (index == conversionTabIdx)
//         conversionScene->wakeUp();
}


void MainWindow::setParam(QString paramDataFrame, QString newParamValue)
{
    QString cmd1 = paramDataFrame + " set " + newParamValue;
    QString cmd2 = QString("xml ") + paramDataFrame + " get ";
    SessionManager::instance()->runCmd({cmd1,cmd2});
}


 void MainWindow::createDockWindows()
{

//     introDock = new QDockWidget("Intro",this);
//     introDock->setWidget(introPanel);
//     addDockWidget(Qt::LeftDockWidgetArea, introDock);
//     viewMenu->addAction(introDock->toggleViewAction());
////     introDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

expertWindow->resize(QApplication::primaryScreen()->availableGeometry().size()*.8);
expertWindow->move(0,0);expertWindow->setWindowState(Qt::WindowMaximized);

expertWindow->hide();
expertLayout=new QHBoxLayout;
expertLayout->addWidget(lazynutPanel);
expertGWidget=new WidgetFwdResizeEvent;
expertGWidget->setLayout(expertLayout);
        expertWindow->setCentralWidget(expertGWidget);
//     codePanelDock = new QDockWidget(tr("lazyNut Code"),this);
 //    codePanelDock->setWidget(lazynutPanel);
//     expertTabWidget->addTab(lazynutPanel,"Code");


     //viewMenu->addAction(codePanelDock->toggleViewAction());
  //   codePanelDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
 //    codePanelDock->hide(); // initially, don't show codePanelDock

     methodsDock = new QDockWidget(tr("Method"), this);
     methodsDock->setWidget(methodsPanel);
     this->addDockWidget(Qt::LeftDockWidgetArea, methodsDock);
     viewMenu->addAction(methodsDock->toggleViewAction());
     methodsDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);


//     diagramDock = new QDockWidget(tr("Architecture"), this);
//     diagramDock->setWidget(diagramWindow);
//     addDockWidget(Qt::RightDockWidgetArea, diagramDock);
//     viewMenu->addAction(diagramDock->toggleViewAction());
//     diagramDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);



//     explorerDock = new QDockWidget("Explorer",this);
//     explorerDock->setWidget(explorerPanel);
//     addDockWidget(Qt::RightDockWidgetArea, explorerDock);
//     viewMenu->addAction(explorerDock->toggleViewAction());
//     explorerDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
//explorerTabIdx=lazynutPanel->addTab("Explorer" );
     resultsDock = new QDockWidget(tr("Results"), this);
     dataframeResultsViewer->hide();
     resultsDock->setWidget(plotViewer);
     addDockWidget(Qt::RightDockWidgetArea, resultsDock);
     viewMenu->addAction(resultsDock->toggleViewAction());
     resultsDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

//     tabifyDockWidget(introDock, codePanelDock);
     //tabifyDockWidget(codePanelDock, methodsDock);
     //tabifyDockWidget(diagramDock, explorerDock);
  //   tabifyDockWidget(explorerDock, resultsDock);
}

void MainWindow::displayExpertWindow()
{
    expertWindow->raise();
    expertWindow->show();
    expertWindow->activateWindow();
    expertWindow->resize(expertWindow->size());
}

void MainWindow::initialiseToolBar()
{
//    QIcon newpix(":/images/zebra_64x64.png");
    QIcon newpix(":/images/expert.mode.jpg");
    expertShow = new QAction(newpix, "E&xpert", this);
    toolbar = addToolBar("main toolbar");
//    QLabel* modelBoxLabel = new QLabel("Model: ");
    modelButton = new QPushButton("Model:");
    modelButton->setFlat(true);
    modelButton->setEnabled(true);

    addonButton = new QToolButton(this);
    addonButton->setIcon(QIcon(":/images/add-on.png"));
    addonButton->setToolTip("load add-on");

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
    connect(addonButton, SIGNAL(clicked()),
              this, SLOT(loadAddOn()));

      modelListFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
      modelComboBox->setModel(modelListFilter);
      modelComboBox->setModelColumn(0);
//      modelComboBox->view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
      modelListFilter->setType("grouping");
      connect(modelListFilter, SIGNAL(objectCreated(QString,QString,QString,QDomDocument*)),
              modelComboBox, SLOT(setCurrentText(QString)));

      trialListFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
      trialComboBox->setModel(trialListFilter);
      trialComboBox->setModelColumn(0);
//      modelComboBox->view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
      trialListFilter->setType("steps");
      connect(trialListFilter, SIGNAL(objectCreated(QString,QString,QString,QDomDocument*)),
              trialComboBox, SLOT(setCurrentText(QString)));



    spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

//    toolbar->addWidget(modelBoxLabel);
    toolbar->addWidget(modelButton);
    // Add values in the combo box
    toolbar->addWidget(modelComboBox);
//    toolbar->addAction(openAct);
//    toolbar->addAction(QIcon(openpix), "Open File");
//    toolbar->addSeparator();


    toolbar->addWidget(addonButton);
    toolbar->addSeparator();


//    toolbar->addWidget(trialBoxLabel);
    toolbar->addWidget(trialButton);
    // Add values in the combo box
    toolbar->addWidget(trialComboBox);

    trialWidget = new TrialWidget(this);
    toolbar->addWidget(trialWidget);
//    connect(trialComboBox,SIGNAL(currentIndexChanged(QString)),trialWidget,SLOT(update(QString)));

//    toolbar->addSeparator();
    stopAct = new QAction("Stop", this);
    stopAct->setToolTip("stop simulation");
    connect(stopAct, SIGNAL(triggered()), SessionManager::instance(), SLOT(oobStop()));
    connect(SessionManager::instance(), &SessionManager::commandsInJob, [=](){setStopButtonIcon(true);});
    connect(SessionManager::instance(), &SessionManager::jobExecuted, [=](){setStopButtonIcon(false);});
    stopButton = new QToolButton(this);
    stopButton->setAutoRaise(true);
    stopButton->setDefaultAction(stopAct);
    stopButton->setIconSize(QSize(40, 40));
    setStopButtonIcon(false);
    stopButton->show();
    expertButton=new QToolButton(this);
    expertButton->setAutoRaise(true);
    expertButton->setDefaultAction(expertShow);
    expertButton->setIconSize(QSize(40,40));


    toolbar->addWidget(stopButton);
    toolbar->addSeparator();

            toolbar->addWidget(expertButton);
            toolbar->addSeparator();

    toolbar->addWidget(spacer);
    toolbar->addSeparator();

    // notifier of run all trial
    runAllTrialLabel = new QLabel;
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
        stopButton->setIcon(QIcon(":/images/stop-disabled.png"));
    });

}

void MainWindow::setStopButtonIcon(bool state)
{
    if (state)
    {
        stopButton->setIcon(QIcon(":/images/stop-enabled.png"));
//        stopButton->setIcon(QIcon(":/images/run_enabled.png"));
        stopButton->setEnabled(true);
    }
    else
    {
        stopButton->setIcon(QIcon(":/images/stop-disabled.png"));
//        stopButton->setIcon(QIcon(":/images/run_disabled.png"));
        stopButton->setEnabled(false);
    }

}

void MainWindow::setQuietMode(bool state)
{
    setQuietModeAct->setChecked(state);
    if (state)
    {
        quietMode = "quietly ";
    }
    else
    {
        quietMode = "";
    }
}

void MainWindow::msgBox(QString msg)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Debug", msg,
                                  QMessageBox::Yes|QMessageBox::No);

}

void MainWindow::setDebugMode(bool isDebugMode)
{
    m_debugMode = isDebugMode;
    if (isDebugMode)
    {
//        diagramPanel->insertTab(conversionTabIdx, conversionPage, "Conversions");
        SessionManager::instance()->runCmd(QString("loglevel debug"));
        setQuietMode(false);
    }
    else
    {
//        diagramPanel->removeTab(conversionTabIdx);
        SessionManager::instance()->runCmd(QString("loglevel info"));
        setQuietMode(true);
    }

    emit debugModeChanged(isDebugMode);
}

void MainWindow::runScript()
{
    scriptEdit->runScript();
}


void MainWindow::loadModel(QString fileName,bool complete)
{
    if (fileName.isEmpty())
    {
        return;
    }
#ifdef WIN32
    if (!SessionManager::instance()->currentModel().isEmpty())
    {
        QProcess::startDetached(
                    QString("%1/easyNet.bat").arg(qApp->applicationDirPath()),
                    QStringList({fileName}),
                    qApp->applicationDirPath());
        diagramPanel->useFake(modelTabIdx,false);
        return;
    }
#endif
    modelScene->goToSleep();
//    conversionScene->goToSleep();

    // load and run script
    qDebug() << "Starting clock ...";
    loadModelTimer.start();
//    loadFile(fileName);

    // the /path/basename is used by DiagramScene objects to load JSON files
    QString base = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName());
    modelScene->setBaseName(base);
//    conversionScene->setBaseName(base);
    setWindowTitle(QFileInfo(fileName).completeBaseName());


    // set up signal - slots so that loadLayout will be called
    // once all of the layers/connections have descriptions
    //        designWindow->prepareToLoadLayout(base);
    //        conversionWindow->prepareToLoadLayout(base);

    // show info page, if there is one
    QString page = QFileInfo(fileName).dir().filePath(QFileInfo(fileName).completeBaseName());
    page.append(".html");
//    if (QFileInfo(page).exists())
//        infoWindow->showInfo(page);

    // note: this synch is wrong, yet it works fine if one loads just one model while
    // no other jobs run. In general commandsCompleted() might be sent from a previous job.
    if(complete)
      connect(SessionManager::instance(),SIGNAL(commandsCompleted()),this,SLOT(afterModelConfig()));
    else
      connect(SessionManager::instance(),SIGNAL(commandsCompleted()),this,SLOT(modelConfigNeeded()));

//    runScript();

    SessionManager::instance()->runCmd(quietMode+QString("include ")+fileName);

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
    QHBoxLayout* h_layout = new QHBoxLayout;
//    h_layout->setMargin( 0 );
    modelChooser  = new QListWidget;
    modelChooser->setFlow(QListView::TopToBottom);
    modelChooser->setViewMode(QListView::IconMode);
    modelChooser->setMovement(QListView::Static);
    modelChooser->resize(QSize(800,670));

    QListWidgetItem* lwi;
    QStringList modelList = QStringList() << "ia" << "bia" << "drc"
                                          << "cdpplus" << "SCM" << "rpm-ia"
                                          << "ltrs" << "PMSP_3_recurrent" << "custom";
    foreach (QString modelName,modelList)
            modelChooser->addItem(lwi = new QListWidgetItem(QIcon(
                                  ":/images/"+modelName+".png"), modelName));

    modelChooser->setIconSize(QSize(250,250));
    modelChooser->show();
    h_layout->addWidget(modelChooser, 0, Qt::AlignHCenter);

    connect(modelChooser, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(modelChooserItemClicked(QListWidgetItem*)));
}

void MainWindow::modelChooserItemClicked(QListWidgetItem* item)
{
    if (item->text()=="custom")
    {
        // bring up file dialog
        QString fileName = QFileDialog::getOpenFileName(this,tr("Load model"),
                                                        SessionManager::instance()->defaultLocation("modelsDir"),
                                                        tr("easyNet Model Files (*.eNm)"));
    //    diagramPanel->hide();
        if(!fileName.isEmpty()) diagramPanel->useFake(modelTabIdx,true);
        loadModel(fileName,true);

    }
    else
    {
        diagramPanel->useFake(modelTabIdx,true);
        loadModel(SessionManager::instance()->defaultLocation("modelsDir")+"/"+item->text()+".eNm",true);
    }

    modelChooser->hide();
}

void MainWindow::loadModelUnconfigured()
{
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load model"),
                                                    SessionManager::instance()->defaultLocation("modelsDir"),
                                                    tr("easyNet Model Files (*.eNm)"));
    if(!fileName.isEmpty()) diagramPanel->useFake(modelTabIdx,true);
    loadModel(fileName,false);
}
void MainWindow::modelConfigNeeded()
{
    LazyNutJob *job = new LazyNutJob;
    job->cmdList << QString("xml %1 list_settings").arg(SessionManager::instance()->currentModel());
    job->setAnswerReceiver(this, SLOT(createModelSettingsDialog(QDomDocument*)), AnswerFormatterType::XML);
    SessionManager::instance()->submitJobs(job);
}

void MainWindow::createModelSettingsDialog(QDomDocument *domDoc)
{
    SettingsForm *form = new SettingsForm(domDoc, this);
    form->setName(SessionManager::instance()->currentModel());
    form->setUseRFormat(false);
    SettingsFormDialog dialog(domDoc, form, QString("Configure model %1").arg(SessionManager::instance()->currentModel()), this);
    dialog.build();
    int result = dialog.exec();
    if (result == QDialog::Accepted)
    {
        LazyNutJob *job = new LazyNutJob;
        job->cmdList << form->getSettingsCmdList(); //.replaceInStrings(QRegExp("^"), QString("%1 setting ")
                                                     //               .arg(SessionManager::instance()->currentModel()));
        job->appendEndOfJobReceiver(this, SLOT(afterModelConfig()));
        SessionManager::instance()->submitJobs(job);
    }
}

void MainWindow::afterModelConfig()
{
    disconnect(SessionManager::instance(),SIGNAL(commandsCompleted()),this,SLOT(modelConfigNeeded()));
    disconnect(SessionManager::instance(),SIGNAL(commandsCompleted()),this,SLOT(afterModelConfig()));
    modelSettingsDisplay->buildForm(SessionManager::instance()->currentModel());
    connect(SessionManager::instance(),SIGNAL(commandsCompleted()),this,SLOT(afterModelStaged()));
    SessionManager::instance()->runCmd(QString("%1%2 stage").arg(quietMode).arg(SessionManager::instance()->currentModel()));
    modelScene->setNewModelLoaded(true);
//    conversionScene->setNewModelLoaded(true);
    diagramSceneTabChanged(diagramPanel->currentIndex());
    modelScene->wakeUp();
    qDebug() << "Time taken to config model:" << QString::number(loadModelTimer.elapsed()) << "ms";
    commandLog->addText(QString("## Time taken to config model:") + QString::number(loadModelTimer.elapsed()) + QString("ms"));


}

void MainWindow::afterModelStaged()
{
    qDebug() << "Time taken to load and stage model:" << QString::number(loadModelTimer.elapsed()) << "ms";
    commandLog->addText(QString("## Time taken to load and stage model:") + QString::number(loadModelTimer.elapsed()) + QString("ms"));
    disconnect(SessionManager::instance(),SIGNAL(commandsCompleted()),this,SLOT(afterModelStaged()));
    connect(modelScene,SIGNAL(animationFinished()),this,SLOT(initialLayout()));
}

void MainWindow::initialLayout()
{
    qDebug()<<"initialLayout";
    diagramPanel->show();
    diagramPanel->useFake(modelTabIdx,false);
    disconnect(modelScene,SIGNAL(animationFinished()),this,SLOT(initialLayout()));

}

void MainWindow::loadTrial()
{
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load trial"),
                                                    SessionManager::instance()->defaultLocation("trialsDir"),
                                                    tr("Script Files (*.eNs *.eNt)"));
    if (!fileName.isEmpty())
    {
        QString easyNetDataHome=SessionManager::instance()->easyNetDataHome();
        int len=easyNetDataHome.length();
        QString fn;
        if(fileName.left(len)==easyNetDataHome)
        {
            if(easyNetDataHome.right(1)!="/") len++;
            fn=fileName.right(fileName.length()-len);
        }
        else fn=fileName;
        QString x=quietMode+"include ";
        x.append(fn);
        SessionManager::instance()->runCmd(x);
    }
    return;

}


void MainWindow::loadAddOn()
{
    if (SessionManager::instance()->currentModel().isEmpty())
        return; // for now we don't want users attempting load add-ons without models
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load add-on"),
                                                    SessionManager::instance()->defaultLocation("trialsDir"),
                                                    "Add-ons (" + SessionManager::instance()->currentModel() + ".*.eNa)");
    if (!fileName.isEmpty())
    {
        QString easyNetDataHome=SessionManager::instance()->easyNetDataHome();
        int len=easyNetDataHome.length();
        QString fn;
        if(fileName.left(len)==easyNetDataHome)
        {
            if(easyNetDataHome.right(1)!="/") len++;
            fn=fileName.right(fileName.length()-len);
        }
        else fn=fileName;
        QString x=quietMode+"include ";
        x.append(fn);
        SessionManager::instance()->runCmd(x);
    }
    return;
}

void MainWindow::loadStimulusSet()
{
    stimSetViewer->open();
    methodsDock->raise();
    methodsPanel->setCurrentIndex(stimSetTabIdx); // show StimSet tab
}

void MainWindow::importDataFrame()
{
    dataframeViewer->open();
//    explorerDock->raise();
//    explorerPanel->setCurrentIndex(dfTabIdx);
}


void MainWindow::updateDFComboBox()
{
    //show new dataframe;
//    explorerDock->raise();
  //  explorerPanel->setCurrentIndex(dfTabIdx);
//    dataframesWindow->selectTable(df_name_for_updating_combobox);
//    disconnect(SessionManager::instance(),SIGNAL(commandsCompleted()),
//                                              this,SLOT(updateDFComboBox()));
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
    QSettings settings("easyNet", "GUI");
    QString easyNetHome, easyNetDataHome;

    easyNetHome = settings.value("easyNetHome").toString();
    if(easyNetHome.isEmpty())
    {
        if (qEnvironmentVariableIsSet("EASYNET_HOME"))
            easyNetHome = QDir::fromNativeSeparators(QString::fromUtf8(qgetenv("EASYNET_HOME")));
    }
    QDir dir(easyNetHome);
    if (easyNetHome.isEmpty() || !dir.exists())
        easyNetHome = "../..";
    SessionManager::instance()->setEasyNetHome(easyNetHome);

    easyNetDataHome = settings.value("easyNetDataHome").toString();
    if(easyNetDataHome.isEmpty())
    {
        if (qEnvironmentVariableIsSet("EASYNET_DATA_HOME"))
            easyNetDataHome = QDir::fromNativeSeparators(QString::fromUtf8(qgetenv("EASYNET_DATA_HOME")));
    }
    dir.setPath(easyNetDataHome);
    if (easyNetDataHome.isEmpty() || !dir.exists())
        easyNetDataHome = easyNetHome;
    SessionManager::instance()->setEasyNetDataHome(easyNetDataHome);

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();

    writeSettings();
}


void MainWindow::writeSettings()
{
    QSettings settings("easyNet", "GUI");
//    settings.setValue("easyNetHome", SessionManager::instance()->easyNetHome());
//    settings.setValue("easyNetDataHome", SessionManager::instance()->easyNetDataHome());
    settings.beginGroup("mainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();
}

bool MainWindow::proceedWithRestartOk()
{
    QMessageBox::StandardButton reply =
            QMessageBox::warning(
                this,
                "Restart simulation engine",
                "The operation you have chosen requires to restart the simulation engine."
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
//        qDebug() << "eNMainWindow emitting saveLayout";
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
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open script"),
                                                        SessionManager::instance()->defaultLocation("scriptsDir"),
                                                        tr("Script Files (*.eNs *.eNm)"));
        if (!fileName.isEmpty())
        {
            loadFile(fileName);
            //codePanelDock->raise();
            lazynutPanel->setCurrentIndex(scriptTabIdx); // show scripts tab
            //codePanelDock->show();
        }
 //   }
}

void MainWindow::runTest()
{
 //   if (maybeSave())
//    {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open script"),
                                                        QString(SessionManager::instance()->defaultLocation("testsDir")).append("//"+modelComboBox->currentText()),
                                                        tr("Script Files (*.eNs)"));
        if (!fileName.isEmpty())
        {
            loadFile(fileName);
            connect(SessionManager::instance(),SIGNAL(commandsCompleted()),this,SLOT(afterTestsCompleted()));
            runScript();
        }
 //   }
}

void MainWindow::afterTestsCompleted()
{
    //codePanelDock->raise();
    lazynutPanel->setCurrentIndex(testsTabIdx);
    displayExpertWindow();
    qDebug() << "trying to change display";
    disconnect(SessionManager::instance(),SIGNAL(commandsCompleted()),this,SLOT(afterTestsCompleted()));

}

void MainWindow::setFormInSettingsWidget(QString name)
{
    QWidget *view = qobject_cast<QWidget *>(sender());
    if (view && !view->visibleRegion().isEmpty())
    {
        dataViewSettingsWidget->setForm(name);
    }
}


void MainWindow::switchFormInSettingsWidget(bool visible)
{
    if (visible)
    {
        QDockWidget *dock = qobject_cast<QDockWidget *>(sender());
        if (dock)
        {
            QTabWidget *panel = qobject_cast<QTabWidget *>(dock->widget());
            if (panel)
                switchFormInSettingsWidget(panel);
        }
    }
}

void MainWindow::switchFormInSettingsWidget(QTabWidget *panel)
{
    if (!panel)
        panel = qobject_cast<QTabWidget *>(sender());
    if (panel)
    {
        DataViewer *viewer = qobject_cast<DataViewer *>(panel->currentWidget());
        if (viewer)
            dataViewSettingsWidget->setForm(viewer->currentItemName());
    }
}

void MainWindow::showResultsViewer(QString name)
{
    QString type = SessionManager::instance()->descriptionCache->type(name);
    resultsDock->raise();
/*    if (type == "dataframe")
    {
        resultsPanel->setCurrentWidget(dataframeResultsViewer);
    }
    else if (type == "xfile")
    {
        resultsPanel->setCurrentWidget(plotViewer);
    }
*/
}


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
//    QSettings settings("easyNet", "GUI");
//    QStringList keys = settings.allKeys();
//    QString details = keys.join("\n");

//    QVector <QLineEdit*> editList;

//    QGroupBox *groupBox = new QGroupBox(tr("Settings"));
//    QFormLayout *formLayout = new QFormLayout;
//    foreach (QString key, keys)
//    {
//        editList.push_back(new QLineEdit(settings.value(key).toString()));
//        formLayout->addRow(key, editList.back());
//    }
//    groupBox->setLayout(formLayout);
//    groupBox->show();

    settingsDialog = new QDialog;
    settingsDialog->setAttribute(Qt::WA_DeleteOnClose);
    QFormLayout *dialogLayout = new QFormLayout;
    foreach(QString env, QStringList({"easyNetHome", "easyNetDataHome"}))
    {
        QLineEdit *lineEdit = new QLineEdit(SessionManager::instance()->easyNetDir(env), settingsDialog);
        lineEdit->setReadOnly(true);
        int pixelsWide = qApp->fontMetrics().width(lineEdit->text()) + 10;
        lineEdit->setMinimumWidth(pixelsWide);
        QPushButton *changeButton = new QPushButton("Change", settingsDialog);
        connect(changeButton, &QPushButton::clicked, [=]() {
            setNewEasyNetDir(env);
            settingsDialog->done(0);
            viewSettings();
        });
        QHBoxLayout *lineLayout = new QHBoxLayout;
        lineLayout->addWidget(lineEdit);
        lineLayout->addWidget(changeButton);
        dialogLayout->addRow(env, lineLayout);
    }
    settingsDialog->setLayout(dialogLayout);
    settingsDialog->adjustSize();
    settingsDialog->exec();
}

void MainWindow::setNewEasyNetHome()
{
    setNewEasyNetDir("easyNetHome");
}

void MainWindow::setNewEasyNetDataHome()
{
   setNewEasyNetDir("easyNetDataHome");
}

void MainWindow::setNewEasyNetDir(QString env)
{
    if (!(env == "easyNetHome" || env == "easyNetDataHome"))
        return;
    if (!proceedWithRestartOk())
        return;
    QString dir = QFileDialog::getExistingDirectory(
                this,
                QString("New %1 directory").arg(env),
                SessionManager::instance()->easyNetDir(env));
    SessionManager::instance()->setEasyNetDir(env, dir);
    SessionManager::instance()->restartLazyNut();
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
    lazyNutConsole->setConsoleFontSize(fontSize);

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
    lazyNutStatusWidget->setCurrentWidget(offLabel);
    QMessageBox::critical(this, "Invalid simulation engine",
            QString("easyNet simulation engine not running or not found.\n"
                    "Please select a valid easyNetHome directory using the menu Settings -> Set easyNetHome directory"));
}

void MainWindow::setLazyNutFinished(bool crashed)
{
    lazyNutStatusWidget->setCurrentWidget(offLabel);
    if (crashed)
    {
        int res = QMessageBox::critical(this, "Simulation engine crash",
                QString("The simulation engine has crashed.\n"
                        "Do you want to restart immediately?\nNote that after restart your current dataframes and plots will be lost"),
                              QMessageBox::Yes |  QMessageBox::Cancel);
        if (res == QMessageBox::Yes )
            SessionManager::instance()->restartLazyNut();
    }
}

void MainWindow::displayVersion(QString version)
{
    QMessageBox::about(this, "Version",QString("LazyNut version is:\n%1").arg(version));
}


void MainWindow::createActions()
{

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

    loadModelUAct = new QAction(tr("Partially load model for configuration"), this);
//    loadModelAct->setShortcuts(QKeySequence::Open);
    loadModelUAct->setStatusTip(tr("Load a previously specified model, pausing to allow settings to be changed"));
    connect(loadModelUAct, SIGNAL(triggered()), this, SLOT(loadModelUnconfigured()));

    modelFinalizeAct = new QAction(tr("Finalize configured model"), this);
//    loadModelAct->setShortcuts(QKeySequence::Open);
    modelFinalizeAct->setStatusTip(tr("Finish loading a model, once settings have been changed"));
    connect(modelFinalizeAct, SIGNAL(triggered()), this, SLOT(afterModelConfig()));

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
    loadScriptAct->setEnabled(true);

    runTestAct = new QAction(QIcon(":/images/code-2x.png"), tr("&Run test"), this);
    runTestAct->setStatusTip(tr("Run a test of this model"));
    connect(runTestAct, SIGNAL(triggered()), this, SLOT(runTest()));
    runTestAct->setEnabled(true);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    restartInterpreterAct = new QAction(tr("Restart simulation engine"), this);
    restartInterpreterAct->setStatusTip(tr("Restart simulation engine"));
    connect(restartInterpreterAct, SIGNAL(triggered()), this, SLOT(restart()));
//    restartInterpreterAct->setDisabled(true);


    viewSettingsAct = new QAction(tr("view/change easyNet settings"), this);
    viewSettingsAct->setStatusTip(tr("View easyNet settings"));
    connect(viewSettingsAct,SIGNAL(triggered()),this, SLOT(viewSettings()));

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

//    assistantAct = new QAction(tr("Help Contents"), this);
//    assistantAct->setShortcut(QKeySequence::HelpContents);
//    connect(assistantAct, SIGNAL(triggered()), this, SLOT(showDocumentation()));

    assistantAct = new QAction(tr("Help Contents"), this);
    assistantAct->setShortcut(QKeySequence::HelpContents);
    connect(assistantAct, SIGNAL(triggered()), this, SLOT(showDocumentation()));

    aboutAct = new QAction(tr("About easyNet"), this);
    connect(aboutAct, &QAction::triggered, [=]()
    {
         QMessageBox::about(this, "easyNet",QString(
                                "<i>easyNet</i> is a software package for computational modelling of cognitive processes.<br>"
                                "<br>"
                                "Authors: James Adelman, Colin Davis, & Michele Gubian<br>"
                                "<br>"
                                "Chief Architect and Programmer of Simulator Engine:<br>James Adelman<br>"
                                "<br>"
                                "Chief Architect of GUI:<br>Colin Davis<br>"
                                "<br>"
                                "Chief Programmer of GUI:<br>Michele Gubian<br>"
                                "<br>"
                                "<i>easyNet</i> is free software licensed under GPLv3<br>"
                                "<br>"
                                "Visit <i>easyNet</i> <a href=\"http://adelmanlab.org/easyNet/\">homepage</a> for more information and latest downloads."));

    });

    setQuietModeAct = new QAction(tr("Quiet mode"), this);
    setQuietModeAct->setCheckable(true);
    connect(setQuietModeAct, SIGNAL(triggered(bool)), this, SLOT(setQuietMode(bool)));
    setQuietModeAct->setChecked(true);

    debugModeAct = new QAction(tr("Debug mode"), this);
    debugModeAct->setCheckable(true);
    connect(debugModeAct, SIGNAL(triggered(bool)), this, SLOT(setDebugMode(bool)));
}

void MainWindow::restart()
{
    if (!proceedWithRestartOk())
        return;
//    if (SessionManager::instance()->isOn())
//    {
//        LazyNutJob *job = new LazyNutJob;
//        job->cmdList << "stop" ;
//        job->appendEndOfJobReceiver(SessionManager::instance(), SLOT(restartLazyNut()));
//        SessionManager::instance()->submitJobs(job);
//    }
//    else
//    {
        SessionManager::instance()->restartLazyNut();
//    }
}

void MainWindow::showDocumentation()
{
    qDebug() << "Entered showDocumentation";
//    assistant->showDocumentation("index.html");

//    introDock->show();
//    introDock->raise();
//    introPanel->setCurrentIndex(infoTabIdx);

//    introPanel->show();
//    introPanel->raise();

//    infoWindow->show();
//    infoWindow->showNormal();
//    infoWindow->raise();

    HelpWindow::showPage("start.html");
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));

    fileSubMenu = fileMenu->addMenu(tr("&Load"));
    fileSubMenu->addAction(loadModelAct);
    fileSubMenu->addAction(loadModelUAct);
    fileSubMenu->addAction(modelFinalizeAct);
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
    fileMenu->addAction(runTestAct);
    fileMenu->addSeparator();
    fileMenu->addAction(restartInterpreterAct);
    fileMenu->addAction(exitAct);

    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    settingsMenu->addAction(viewSettingsAct);
    settingsSubMenu = settingsMenu->addMenu(tr("&Font size"));
    settingsSubMenu->addAction(setSmallFontAct);
    settingsSubMenu->addAction(setMediumFontAct);
    settingsSubMenu->addAction(setLargeFontAct);
    settingsMenu->addSeparator();
    settingsMenu->addAction(debugModeAct);
    settingsMenu->addAction(setQuietModeAct);


//    settingsMenu->addAction(synchModeAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    menuBar()->addSeparator();

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(assistantAct);
    helpMenu->addAction(aboutAct);
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
    connect(SessionManager::instance(), SIGNAL(lazyNutNotRunning()), this, SLOT(lazyNutNotRunning()));
    connect(SessionManager::instance(), SIGNAL(lazyNutFinished(bool)), this, SLOT(setLazyNutFinished(bool)));


    lazyNutCmdLabel = new QLabel;
    lazyNutCmdLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    lazyNutCmdLabel->setFixedWidth(qApp->fontMetrics().width("LAST COMMAND: a very very very very long lazyNut command"));
    lazyNutCmdLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    statusBar()->addWidget(lazyNutCmdLabel, 1);
    connect(SessionManager::instance(), SIGNAL(commandExecuted(QString,QString)),
            this, SLOT(showCmdOnStatusBar(QString)));

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
    static QString format=QString("LAST COMMAND: %1");
    lazyNutCmdLabel->setText(format.arg(cmd));
}

void MainWindow::addOneToLazyNutProgressBar()
{
    lazyNutProgressBar->setValue(lazyNutProgressBar->value()+1);
}

void MainWindow::updateTrialRunListCount(int count)
{
    QString progressText = m_trialListLength > 0 ? QString("%1 of %2").arg(count).arg(m_trialListLength) : QString::number(count);
    runAllTrialLabel->setText(QString("Processing a trial list\nProgress: %1").arg(progressText));
    if (m_trialListLength == count)
        m_trialListLength = 0;
}

void MainWindow::processHistoryKey(int dir, QString text)
{
    QString line = commandLog->getHistory(dir, text);
    emit showHistory(line);
}

void MainWindow::showModelSettings()
{
    modelSettingsDialog->show();
    modelSettingsDialog->showNormal();
    modelSettingsDialog->raise();

}

void MainWindow::showParameterSettings()
{
    paramSettingsDialog->show();
    paramSettingsDialog->showNormal();
    paramSettingsDialog->raise();

}


void MainWindow::showDataViewSettings()
{
    dataViewSettingsDialog->show();
    dataViewSettingsDialog->showNormal();
    dataViewSettingsDialog->raise();

}

void MainWindow::setFormAndShow(QString name)
{
    if (name.isEmpty())
    {
        LazyNutJob *job = qobject_cast<LazyNutJob *>(sender());
        if (!job)
        {
            eNerror << "cannot extract LazyNutJob from sender";
            return;
        }
        name = SessionManager::instance()->getDataFromJob(job, "name").toString();
        if (name.isEmpty())
        {
            eNerror << "LazyNutJob->data contains an empty name entry";
            return;
        }
    }
    showDataViewSettings();
    dataViewSettingsWidget->setForm(name);
}
