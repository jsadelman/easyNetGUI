#include "tablewindow.h"
#include "xmlaccessor.h"
#include "sessionmanager.h"
#include "lazynutjob.h"
#include "tabstablewidget.h"
#include "objectcache.h"
#include "objectcachefilter.h"
#include "trialdataframemodel.h"

#include <QMenu>
#include <QSignalMapper>
#include <QDomDocument>
#include <QToolBar>
#include <QDebug>


TableWindow::TableWindow(QWidget *parent)
    : tableCounter(0), ResultsWindow_If(parent)
{
    createActions();
    createMenus();
    createToolBars();
    // trial dispatch defaults
    setSingleTrialMode(Append);
    setSingleTrialDispatchModeActs.at(Append)->setChecked(true);
    setTrialListMode(New);
    setTrialListDispatchModeActs.at(New)->setChecked(true);
    setDispatchModeAuto(true);
    setDispatchModeAutoAct->setChecked(true);

    // instantiate main widget
    tableWidget = new TabsTableWidget(this);
    setCentralWidget(tableWidget);
    // setup pretty headers (not the right place here, it should be customisable from this class
    tableWidget->addHeaderReplaceRules(Qt::Horizontal,"event_pattern", "");
    tableWidget->addHeaderReplaceRules(Qt::Horizontal,"\\(", "");
    tableWidget->addHeaderReplaceRules(Qt::Horizontal,"\\)", "");

    dataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    dataframeFilter->setType("dataframe");
    connect(dataframeFilter, SIGNAL(objectDestroyed(QString)), this, SLOT(removeTable(QString)));
}

TableWindow::~TableWindow()
{
}

void TableWindow::open()
{

}

void TableWindow::save()
{

}

void TableWindow::copy()
{

}

void TableWindow::addTable()
{
    LazyNutJob *job = qobject_cast<LazyNutJob *>(sender());
    if (!job)
    {
        qDebug() << "ERROR: TableWindow::addTable cannot extract LazyNutJob from sender";
        return;
    }
    QMap<QString, QVariant> data = job->data.toMap();
    if (!data.contains("addTable"))
    {
        qDebug() << "ERROR: TableWindow::addTable LazyNutJob->data does not contain addTable entry";
        return;
    }
    QString tableName = data.value("addTable").toString();
    if (tableName.isEmpty())
    {
        qDebug() << "ERROR: TableWindow::addTable LazyNutJob->data addTable entry is empty";
        return;
    }
    if (!SessionManager::instance()->descriptionCache->exists(tableName))
    {
        qDebug() << QString("ERROR: TableWindow::addTable attempt to add a non-existing table (dataframe) %1")
                    .arg(tableName);
        return;
    }
    tableWidget->addTable(tableName);
}

void TableWindow::removeTable(QString name)
{
    // assumes that name is the name of a destination table deleted by the user,
    // not of a results dataframe
    // e.g. Table_1, as opposed to ((ldt default_observer) default_dataframe)
    QMutableMapIterator<QString, QString> dispatchMapIt(dispatchMap);
    while (dispatchMapIt.hasNext())
    {
        dispatchMapIt.next();
        if (dispatchMapIt.value() == name)
            dispatchMapIt.remove();
    }
    trialRunInfoMap.remove(name);
}


void TableWindow::createActions()
{
    ResultsWindow_If::createActions();

    openAct->setStatusTip(tr("Open an existing table"));
    saveAct->setStatusTip(tr("Save current table"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));

    copyDFAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy to new dataframe"), this);
    copyDFAct->setStatusTip(tr("Copy contents to a new dataframe "));
//    connect(copyDFAct, SIGNAL(triggered()), this, SLOT(on_copy_DF_clicked()));

    mergeDFAct = new QAction(QIcon(":/images/Merge_Icon.png"), tr("&Merge two dataframes"), this);
    mergeDFAct->setStatusTip(tr("Merge two dataframes"));
//    connect(mergeDFAct, SIGNAL(triggered()), this, SLOT(mergeFD()));

    findAct = new QAction(QIcon(":/images/magnifying-glass-2x.png"), tr("&Find"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setToolTip(tr("Find text in this table"));
//    connect(findAct, SIGNAL(triggered()), this, SLOT(showFindDialog()));

    plotAct = new QAction(QIcon(":/images/barchart2.png"), tr("&Plot"), this);
    plotAct->setToolTip(tr("Create a plot based on these data"));
//    connect(plotAct, SIGNAL(triggered()), this, SLOT(preparePlot()));

}

void TableWindow::createMenus()
{
    ResultsWindow_If::createMenus();

    editMenu->addAction(copyDFAct);
    editMenu->addAction(mergeDFAct);
    editMenu->addAction(findAct);
    editMenu->addAction(plotAct);

}

void TableWindow::createToolBars()
{
    ResultsWindow_If::createToolBars();

    editToolBar->addAction(copyDFAct);
    editToolBar->addAction(mergeDFAct);
    editToolBar->addAction(findAct);
    editToolBar->addAction(plotAct);
}

void TableWindow::dispatch_Impl(QDomDocument *info)
{
    QDomElement rootElement = info->documentElement();
    QDomElement runModeElement = XMLAccessor::childElement(rootElement, "Run mode");
    QDomElement resultsElement = XMLAccessor::childElement(rootElement, "Results");
    QString runMode = XMLAccessor::value(runModeElement);
    QString results = XMLAccessor::value(resultsElement);

    int currentDispatchMode;
    if (!dispatchModeAuto)
        currentDispatchMode = dispatchModeOverride;
    else if (runMode == "single")
        currentDispatchMode = singleTrialDispatchMode;
    else if (runMode == "list")
        currentDispatchMode = trialListDispatchMode;
    else
    {
        qDebug() << "ERROR: TableWindow::dispatch_Impl cannot read trial run info XML.";
        return;
    }
    int action;
    if (!dispatchMap.contains(results))
        action = New;
    else if (!dispatchModeAuto)
        action = currentDispatchMode;
    else
        action = dispatchFST.value(qMakePair(dispatchModeMap.value(results, New), currentDispatchMode));
    QString dispatchDestination;
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->updateObjectCatalogueJobs();
    switch(action)
    {
    case New:
    {
        dispatchDestination = newTableName();
        job->cmdList << QString("%1 copy %2").arg(results).arg(dispatchDestination);
        QMap<QString, QVariant> data;
        data.insert("addTable", dispatchDestination);
        jobs.last()->data = data;
        jobs.last()->appendEndOfJobReceiver(this, SLOT(addTable()));
        break;
    }
    case Overwrite:
    {
        dispatchDestination = dispatchMap.value(results);
        job->cmdList << QString("R << eN[\"%1\"] <- eN[\"%2\"]").arg(dispatchDestination).arg(results);
        break;
    }
    case Append:
    {
        dispatchDestination = dispatchMap.value(results);
        job->cmdList << QString("R << eN[\"%1\"] <- rbind(eN[\"%1\"],eN[\"%2\"])").arg(dispatchDestination).arg(results);
        break;
    }
    default:
    {
        qDebug() << "TableWindow::dispatch_Impl computed action was unrecognised";
        foreach(LazyNutJob *j, jobs)
            delete j;
        return;
    }
    }
    SessionManager::instance()->submitJobs(jobs);
    // update maps
    dispatchMap[results] = dispatchDestination;
    dispatchModeMap[results] = currentDispatchMode;
    trialRunInfoMap[dispatchDestination].append(info);
}

QString TableWindow::newTableName()
{
    return QString("Table_%1").arg(++tableCounter);
}

