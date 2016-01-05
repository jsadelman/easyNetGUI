#include "tablewindow.h"
#include "xmlaccessor.h"
#include "sessionmanager.h"
#include "lazynutjob.h"
#include "tabstablewidget.h"
#include "objectcache.h"
#include "objectcachefilter.h"
#include "trialdataframemodel.h"
#include "xmlform.h"
#include "settingsform.h"
#include "settingsformdialog.h"
#include "easyNetMainWindow.h"


#include <QMenu>
#include <QSignalMapper>
#include <QDomDocument>
#include <QToolBar>
#include <QDebug>
#include <QDockWidget>
#include <QScrollArea>
#include <QSettings>
#include <QFile>


TableWindow::TableWindow(QWidget *parent)
    : tableCounter(0), lastOpenDir(""), lastSaveDir(""), lastResults(""), ResultsWindow_If(parent)
{
    // instantiate main widget
    tableWidget = new TabsTableWidget(this);
    setCentralWidget(tableWidget);

    connect(tableWidget, &TabsTableWidget::currentTableChanged, [=](QString name)
    {
       if (infoVisible)
           showInfo(name);
    });

    dataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    dataframeFilter->setType("dataframe");
    connect(dataframeFilter, SIGNAL(objectDestroyed(QString)), this, SLOT(removeTable(QString)));


    createActions();
    createToolBars();
    connect(tableWidget, SIGNAL(hasCurrentTable(bool)), this, SLOT(enableActions(bool)));
    enableActions(false);
    // trial dispatch defaults
    setSingleTrialMode(Dispatch_Append);
    setTrialListMode(Dispatch_New);
}

TableWindow::~TableWindow()
{
}

void TableWindow::setPrettyHeaders(QString trial, QString table)
{
    TrialDataFrameModel *prettyHeadersModel = new TrialDataFrameModel(this);
    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal, "event_pattern", "");
    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal,"\\(", "");
    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal,"\\)", "");
    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal,trial, "");
    tableWidget->setPrettyHeaders(table, prettyHeadersModel);
}

void TableWindow::preDispatch(QDomDocument *info)
{
    QDomElement rootElement = info->documentElement();
    QDomElement resultsElement = XMLAccessor::childElement(rootElement, "Results");
    QString results = XMLAccessor::value(resultsElement);
    QDomElement trialElement = XMLAccessor::childElement(rootElement, "Trial");
    QString trial = XMLAccessor::value(trialElement);
    QDomElement runModeElement = XMLAccessor::childElement(rootElement, "Run mode");
    QString runMode = XMLAccessor::value(runModeElement);
    int currentDispatchMode;
    if (runMode == "single")
        currentDispatchMode = singleTrialDispatchMode;
    else if (runMode == "list")
        currentDispatchMode = trialListDispatchMode;
    else
    {
        qDebug() << "ERROR: TableWindow::dispatch_Impl cannot read trial run info XML.";
        return;
    }

    int action;
    if (!tableWidget->contains(results))
    {
        action = Dispatch_Overwrite;
    }
    else if (!dispatchModeAuto && dispatchModeOverride > -1)
    {
        action = dispatchModeOverride;
    }
    else
    {
        QDomElement resultsRoot = trialRunInfoMap[results]->documentElement();
        QDomElement previousRunModeElement = XMLAccessor::childElement(resultsRoot, "Run mode");
        QString previousRunMode = XMLAccessor::value(previousRunModeElement);
        int previousDispatchMode;
        if (previousRunMode == "single")
            previousDispatchMode = singleTrialDispatchMode;
        else if (previousRunMode == "list")
            previousDispatchMode = trialListDispatchMode;
        else
        {
            qDebug() << "ERROR: TableWindow::preDispatch cannot read trial run info XML.";
            return;
        }
        action = dispatchModeFST.value(qMakePair(previousDispatchMode, currentDispatchMode));
    }
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList = QStringList();
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>() << job;
    switch(action)
    {
    case Dispatch_New:
    {
        QDomDocument* description = SessionManager::instance()->descriptionCache->getDomDoc(results);
        QDomElement rootElement = description->documentElement();
        QDomElement prettyNameElement = XMLAccessor::childElement(rootElement, "pretty name");
        QString prettyName = XMLAccessor::value(prettyNameElement);
        QString backupTable = SessionManager::instance()->makeValidObjectName(prettyName);
        job->cmdList << QString("%1 copy %2").arg(results).arg(backupTable);
        job->cmdList << QString("%1 clear").arg(results);
        QMap<QString, QVariant> data;
        data.insert("addTable", backupTable);
        jobs << SessionManager::instance()->recentlyCreatedJob();
        jobs.last()->data = data;
        jobs.last()->appendEndOfJobReceiver(this, SLOT(addTable()));
        trialRunInfoMap[backupTable] = trialRunInfoMap[results];
        setPrettyHeaders(trial, backupTable);
        break;
    }
    case Dispatch_Overwrite:
    {
        job->cmdList << QString("%1 clear").arg(results);
        if (!tableWidget->contains(results))
        {
            tableWidget->addTable(results);
            setPrettyHeaders(trial, results);
        }
        break;
    }
    case Dispatch_Append:
    {
        // don't send results clear
        break;
    }
    default:
    {
        qDebug() << "TableWindow::dispatch_Impl computed action was unrecognised";
    }
    }
    tableWidget->setTabState(lastResults, TabsTableWidget::Tab_DefaultState);
    tableWidget->setTabState(results, TabsTableWidget::Tab_Updating);
    if (!job->cmdList.isEmpty())
    {
        SessionManager::instance()->submitJobs(jobs);
    }
    else
    {
        foreach(LazyNutJob *j, jobs)
            delete j;
    }
}

void TableWindow::enableActions(bool enable)
{
    // all actions except for open are disabled when there are no tables
    saveAct->setEnabled(enable);
    copyAct->setEnabled(enable);
    copyDFAct->setEnabled(enable);
    dataframeMergeAct->setEnabled(enable);
    findAct->setEnabled(enable);
    plotAct->setEnabled(enable);
    setDispatchModeAutoAct->setEnabled(enable);
    infoAct->setEnabled(enable);
}

void TableWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Import dataframe"),
                                                    lastOpenDir.isEmpty() ? MainWindow::instance()->stimDir : lastOpenDir,
                                                    tr("Database Files (*.eNd);;Text files (*.csv);;All files (*.*)"));
    if (!fileName.isEmpty())
    {
        // create db
        QFileInfo fi(fileName);
        QString tableName = fi.completeBaseName();
        lastOpenDir = fi.path();

        fileName = QDir(MainWindow::instance()->easyNetDataHome).relativeFilePath(fileName);
        QString loadCmd = fi.suffix() == "csv" ? "load_csv" : "load";
        LazyNutJob *job = new LazyNutJob;
        job->logMode |= ECHO_INTERPRETER;
        job->cmdList = QStringList({
                            QString("create dataframe %1").arg(tableName),
                            QString("%1 %2 %3").arg(tableName).arg(loadCmd).arg(fileName)});
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job
                << SessionManager::instance()->recentlyCreatedJob();
        QMap<QString, QVariant> data;
        data.insert("addTable", tableName);
        jobs.last()->data = data;
        jobs.last()->appendEndOfJobReceiver(this, SLOT(addTable()));
        SessionManager::instance()->submitJobs(jobs);
    }
}

void TableWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Save as CSV file"),
                        lastSaveDir.isEmpty() ? QString("%1/Databases").arg(MainWindow::instance()->easyNetDataHome) : lastOpenDir,
                        "CSV (*.csv)");
    if (!fileName.isEmpty())
    {
        LazyNutJob *job = new LazyNutJob;
        job->logMode |= ECHO_INTERPRETER;
        job->cmdList = QStringList({QString("%1 save_csv %2").arg(tableWidget->currentTable()).arg(fileName)});
        SessionManager::instance()->submitJobs(job);
        lastSaveDir = QFileInfo(fileName).path();
    }
}

void TableWindow::copy()
{
    // this is an illegal approach -- get R to copy the df to the clipboard
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList = QStringList({QString("R << write.table(eN[\"%1\"], \"clipboard\", sep=\"\t\", row.names=FALSE)")
                                .arg(tableWidget->currentTable())});
    SessionManager::instance()->submitJobs(job);
}

void TableWindow::copyDF()
{
    QString originalTable = tableWidget->currentTable();
    QString copyTable = newTableName();
    QString trial;
    if (trialRunInfoMap.contains(originalTable))
    {
        QDomElement rootElement = trialRunInfoMap.value(originalTable)->documentElement();
        QDomElement trialElement = XMLAccessor::childElement(rootElement, "Trial");
        trial = XMLAccessor::value(trialElement);

        trialRunInfoMap[copyTable] = trialRunInfoMap[originalTable];
    }
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList << QString("%1 copy %2").arg(originalTable).arg(copyTable);
    QMap<QString, QVariant> data;
    data.insert("addTable", copyTable);
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->recentlyCreatedJob();
    jobs.last()->data = data;
    jobs.last()->appendEndOfJobReceiver(this, SLOT(addTable()));
    SessionManager::instance()->submitJobs(jobs);

    setPrettyHeaders(trial, copyTable);
}

void TableWindow::preparePlot()
{
    if (tableWidget->currentTable().isEmpty())
        return;
    QMap<QString,QString> settings;
    settings["df"] = tableWidget->currentTable();
    QString plotName = SessionManager::instance()->makeValidObjectName(tableWidget->currentTable().append(".plot"));
    QString plotType = "plot_mean_bars.R"; // testing!!!
    emit createNewRPlot(plotName, plotType, settings, 0);
    emit showPlotSettings();
}

void TableWindow::dataframeMerge()
{
    // load XML
    QDomDocument* domDoc = new QDomDocument;
    QSettings settings("QtEasyNet", "nmConsole");
    QString easyNetHome = settings.value("easyNetHome","../..").toString();
    QFile file(QString("%1/XML_files/dataframe_merge.xml").arg(easyNetHome));
    if (!file.open(QIODevice::ReadOnly))
        return;
    if (!domDoc->setContent(&file)) {
        file.close();
        return;
    }
    file.close();
    // setup form
    SettingsForm *form = new SettingsForm(domDoc, this);
    form->setUseRFormat(false);
    QMap<QString, QString> preFilledSettings;
    preFilledSettings["x"] = tableWidget->currentTable();
    preFilledSettings["y"] = SessionManager::instance()->currentSet();
    form->setDefaultSettings(preFilledSettings);
    // setup dialog
    QString info("Select two dataframes you want to merge into one. Their key columns should match.");
    SettingsFormDialog dialog(domDoc, form, info, this);


    connect(&dialog, &SettingsFormDialog::dataframeMergeSettingsReady,
            [=](QStringList cmdList, QString dfName, QString x, QString y)
    {
        LazyNutJob *job = new LazyNutJob;
        job->logMode |= ECHO_INTERPRETER;

        job->cmdList = cmdList;
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job
                << SessionManager::instance()->recentlyCreatedJob();
        QMap<QString, QVariant> data;
        data.insert("addTable", dfName);
        jobs.last()->data = data;
        jobs.last()->appendEndOfJobReceiver(this, SLOT(addTable()));
        jobs.last()->appendEndOfJobReceiver(this, SLOT(refreshInfo()));
        SessionManager::instance()->submitJobs(jobs);
        // pass info over, assume only one of the two source df is a result
        if (trialRunInfoMap.contains(x))
        {
            trialRunInfoMap[dfName] = trialRunInfoMap[x];
        }
        else if (trialRunInfoMap.contains(y))
        {
            trialRunInfoMap[dfName] = trialRunInfoMap[y];
        }
//        emit addDataframeMerge(x, dfName);
//        emit addDataframeMerge(y, dfName);
        SessionManager::instance()->addDataframeMerge(x, dfName);
        SessionManager::instance()->addDataframeMerge(y, dfName);
    });

    dialog.exec();
}

void TableWindow::addTable()
{
    QVariant v = SessionManager::instance()->getDataFromJob(sender(), "addTable");
    if (!v.canConvert<QString>())
    {
        qDebug() << "ERROR: TableWindow::addTable cannot retrieve a valid string from addTable key in sender LazyNut job";
    }
    QString tableName = v.value<QString>();
    if (tableName.isEmpty())
    {
        qDebug() << "ERROR: TableWindow::addTable string from addTable key in sender LazyNut job is empty";
    }
    else if (!SessionManager::instance()->descriptionCache->exists(tableName))
    {
        qDebug() << QString("ERROR: TableWindow::addTable attempt to add a non-existing table (dataframe) %1")
                    .arg(tableName);
    }
    else
    {
        tableWidget->addTable(tableName);
    }
}

void TableWindow::removeTable(QString name)
{
    // assumes that name is the name of a destination table deleted by the user,
    // not of a results dataframe
    // e.g. Table_1, as opposed to ((ldt default_observer) default_dataframe)
//    QMutableMapIterator<QString, QSet<QString> > dispatchMapIt(dispatchMap);
//    while (dispatchMapIt.hasNext())
//    {
//        dispatchMapIt.next();
//        dispatchMapIt.value().remove(name);
//        if (dispatchMapIt.value().isEmpty())
//            dispatchMapIt.remove();
//    }
    trialRunInfoMap.remove(name);
//    sourceDfMap.remove(name);
    if (tableWidget->currentTable().isEmpty())
        delete infoScroll->takeWidget();
}

void TableWindow::setInfoVisible(bool visible)
{
    infoVisible = visible;
    if (infoVisible)
        showInfo(tableWidget->currentTable());
    else
        hideInfo();
}

void TableWindow::refreshInfo()
{
    if (infoVisible)
        showInfo(tableWidget->currentTable());
}


void TableWindow::createActions()
{
    ResultsWindow_If::createActions();

    openAct->setStatusTip(tr("Open an existing table"));
    saveAct->setStatusTip(tr("Save current table as CSV"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));

    copyDFAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy to new dataframe"), this);
    copyDFAct->setStatusTip(tr("Copy contents to a new dataframe"));
    connect(copyDFAct, SIGNAL(triggered()), this, SLOT(copyDF()));

    dataframeMergeAct = new QAction(QIcon(":/images/Merge_Icon.png"), tr("&Merge two dataframes"), this);
    dataframeMergeAct->setStatusTip(tr("Merge two dataframes"));
    connect(dataframeMergeAct, SIGNAL(triggered()), this, SLOT(dataframeMerge()));

    findAct = new QAction(QIcon(":/images/magnifying-glass-2x.png"), tr("&Find"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setToolTip(tr("Find text in this table"));
//    connect(findAct, SIGNAL(triggered()), this, SLOT(showFindDialog()));

    plotAct = new QAction(QIcon(":/images/barchart2.png"), tr("&Plot"), this);
    plotAct->setToolTip(tr("Create a plot based on these data"));
    connect(plotAct, SIGNAL(triggered()), this, SLOT(preparePlot()));


}


void TableWindow::createToolBars()
{
    ResultsWindow_If::createToolBars();

    editToolBar->addAction(copyDFAct);
    editToolBar->addAction(dataframeMergeAct);
    editToolBar->addAction(findAct);
    editToolBar->addAction(plotAct);

}

void TableWindow::dispatch_Impl(QDomDocument *info)
{
    QDomElement rootElement = info->documentElement();
    QDomElement resultsElement = XMLAccessor::childElement(rootElement, "Results");
    QString results = XMLAccessor::value(resultsElement);
    tableWidget->setTabState(results, TabsTableWidget::Tab_Ready);
    lastResults = results;

    trialRunInfoMap[results] = info;
    // should be assigned also to merged dfs that have results as source
    refreshInfo();
}

void TableWindow::showInfo(QString name)
{
    if (!trialRunInfoMap.contains(name))
    {
//        qDebug () << "ERROR: TableWindow::showInfo no info available for table" << name;
        return;
    }
    XMLForm *infoForm = new XMLForm(trialRunInfoMap[name]->documentElement());
    infoForm->build();
    infoScroll->setWidget(infoForm);
    infoForm->show();
    // leakage
}

void TableWindow::hideInfo()
{

}

QString TableWindow::newTableName()
{
    return QString("Table_%1").arg(++tableCounter);
}

