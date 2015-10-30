#include "tablewindow.h"
#include "xmlaccessor.h"
#include "sessionmanager.h"
#include "lazynutjob.h"
#include "tabstablewidget.h"
#include "objectcache.h"
#include "objectcachefilter.h"
#include "trialdataframemodel.h"
#include "xmlform.h"

#include <QMenu>
#include <QSignalMapper>
#include <QDomDocument>
#include <QToolBar>
#include <QDebug>
#include <QDockWidget>
#include <QScrollArea>


TableWindow::TableWindow(QWidget *parent)
    : tableCounter(0), ResultsWindow_If(parent)
{
    // instantiate main widget
    tableWidget = new TabsTableWidget(this);
    setCentralWidget(tableWidget);
    // setup pretty headers (not the right place here, it should be customisable from this class
    tableWidget->addHeaderReplaceRules(Qt::Horizontal,"event_pattern", "");
    tableWidget->addHeaderReplaceRules(Qt::Horizontal,"\\(", "");
    tableWidget->addHeaderReplaceRules(Qt::Horizontal,"\\)", "");
    connect(tableWidget, &TabsTableWidget::currentTableChanged, [=](QString name)
    {
       if (infoVisible)
           showInfo(name);
    });

    // info
    infoDock = new QDockWidget("Info",this);
    infoScroll = new QScrollArea(this);
    infoScroll->setWidgetResizable(true);
    infoDock->setWidget(infoScroll);
    addDockWidget(Qt::BottomDockWidgetArea, infoDock);
    infoDock->setFeatures(QDockWidget::DockWidgetClosable);

    dataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    dataframeFilter->setType("dataframe");
    connect(dataframeFilter, SIGNAL(objectDestroyed(QString)), this, SLOT(removeTable(QString)));

    createActions();
    createToolBars();
    // trial dispatch defaults
    setSingleTrialMode(Append);
    setTrialListMode(New);
    setDispatchModeAuto(true);
    setDispatchModeAutoAct->setChecked(true);
//    infoAct->setChecked(false);

    infoVisible = false;
    infoDock->close();
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
    QMutableMapIterator<QString, QSet<QString> > dispatchMapIt(dispatchMap);
    while (dispatchMapIt.hasNext())
    {
        dispatchMapIt.next();
        dispatchMapIt.value().remove(name);
        if (dispatchMapIt.value().isEmpty())
            dispatchMapIt.remove();
    }
    trialRunInfoMap.remove(name);
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

    infoAct = infoDock->toggleViewAction();
    infoAct->setIcon(QIcon(":/images/Information-icon.png"));
    infoAct->setText(tr("&Info"));
    infoAct->setToolTip(tr("Show/hide info about this table"));
//    infoAct->setCheckable(true);
    connect(infoAct, SIGNAL(triggered(bool)), this, SLOT(setInfoVisible(bool)));

}


void TableWindow::createToolBars()
{
    ResultsWindow_If::createToolBars();

    editToolBar->addAction(copyDFAct);
    editToolBar->addAction(mergeDFAct);
    editToolBar->addAction(findAct);
    editToolBar->addAction(plotAct);

    infoToolBar = addToolBar(tr("Info"));
    infoToolBar->addAction(infoAct);
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
    int action = dispatchMap.contains(results) ? currentDispatchMode : New;

    QString dispatchDestination = QString(); // if not assigned, it will be newTableName();
    switch(action)
    {
    case New:
    {
        // new table
        break;
    }
    case Append:
    case Overwrite:
    {
        if (dispatchModeAuto)
        {
            // destination is a table in Append or Overwrite mode if it's there,
            // otherwise a new one
            foreach(QString table, dispatchMap[results])
            {
                if (dispatchModeMap.value(table) == Append ||
                    dispatchModeMap.value(table) == Overwrite)
                {
                    dispatchDestination = table;
                    break;
                }
            }
        }
        else
            dispatchDestination = tableWidget->currentTable();
        break;
    }
    default:
    {
        qDebug() << "TableWindow::dispatch_Impl computed action was unrecognised";
        return;
    }
    }
    if (dispatchDestination.isEmpty())
        dispatchDestination = newTableName();

    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->updateObjectCatalogueJobs();
    jobs.last()->appendEndOfJobReceiver(this, SLOT(refreshInfo()));
    switch(action)
    {
    case New:
    {
        job->cmdList << QString("%1 copy %2").arg(results).arg(dispatchDestination);
        QMap<QString, QVariant> data;
        data.insert("addTable", dispatchDestination);
        jobs.last()->data = data;
        jobs.last()->appendEndOfJobReceiver(this, SLOT(addTable()));
        break;
    }
    case Overwrite:
    {
        job->cmdList << QString("R << eN[\"%1\"] <- eN[\"%2\"]").arg(dispatchDestination).arg(results);
        break;
    }
    case Append:
    {
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
    dispatchMap[results].insert(dispatchDestination);
    dispatchModeMap[dispatchDestination] = currentDispatchMode;
    trialRunInfoMap[dispatchDestination].append(info);
}

void TableWindow::showInfo(QString name)
{
    if (!trialRunInfoMap.contains(name) || trialRunInfoMap.value(name).isEmpty())
    {
//        qDebug () << "ERROR: TableWindow::showInfo no info available for table" << name;
        return;
    }
    XMLForm *infoForm = new XMLForm(trialRunInfoMap[name].last()->documentElement());
    infoForm->build();
    infoScroll->setWidget(infoForm);
    infoForm->show();
}

void TableWindow::hideInfo()
{

}

QString TableWindow::newTableName()
{
    return QString("Table_%1").arg(++tableCounter);
}

