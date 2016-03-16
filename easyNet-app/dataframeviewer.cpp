#include "dataframeviewer.h"
#include "easyNetMainWindow.h"
#include "lazynutjob.h"
#include "enumclasses.h"
#include "sessionmanager.h"
#include "prettyheadersmodel.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "dataframemodel.h"
#include "ui_dataviewer.h"
#include "trialwidget.h"
#include "finddialog.h"
#include "dataviewerdispatcher.h"
#include "settingsform.h"
#include "dataframemergesettingsformdialog.h"
#include "xmlelement.h"


#include <QSettings>
#include <QFileDialog>
#include <QTableView>

Q_DECLARE_METATYPE(QSharedPointer<QDomDocument> )


DataframeViewer::DataframeViewer(Ui_DataViewer *ui, QWidget *parent)
    : DataViewer(ui, parent), m_dragDropColumns(false), m_stimulusSet(false), m_parametersTable(false)
{
    dataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    dataframeUpdater = new ObjectUpdater(this);
    dataframeUpdater->setCommand("get");
    dataframeUpdater->setProxyModel(dataframeFilter);
    connect(dataframeUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)),
            this, SLOT(updateDataframe(QDomDocument*,QString)));

//    dataframeDescriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
//    dataframeDescriptionUpdater = new ObjectUpdater(this);
//    dataframeDescriptionUpdater->setCommand("description");
//    dataframeDescriptionUpdater->setProxyModel(dataframeDescriptionFilter);
    findDialog = new FindDialog(this);
    findDialog->hideExtendedOptions();
    connect(findDialog, SIGNAL(findForward(QString, QFlags<QTextDocument::FindFlag>)),
            this, SLOT(findForward(QString, QFlags<QTextDocument::FindFlag>)));
    requestedDataframeViews.clear();

    addExtraActions();
}

void DataframeViewer::setParametersTable(bool isParametersTable)
{
    m_parametersTable = isParametersTable;
    ui->settingsAct->setVisible(!isParametersTable);
    plotButton->setVisible(!isParametersTable);
    dataframeViewButton->setVisible(!isParametersTable);
    emit parametersTableChanged(isParametersTable);
}



void DataframeViewer::open()
{

    QString fileName = QFileDialog::getOpenFileName(this,tr("Import dataframe"),
                                                    lastOpenDir.isEmpty() ? defaultOpenDir : lastOpenDir,
                                                    tr("Database Files (*.eNd);;Text files (*.csv);;All files (*.*)"));
    if (!fileName.isEmpty())
    {
        // create db
        QFileInfo fi(fileName);
        QString dfName = SessionManager::instance()->makeValidObjectName(fi.completeBaseName());
        lastOpenDir = fi.path();

        fileName = QDir(SessionManager::instance()->easyNetDataHome()).relativeFilePath(fileName);
        QString loadCmd = fi.suffix() == "csv" ? "load_csv" : "load";
        QString dataframeType = stimulusSet() ? "stimulus_set" : "dataframe";
        LazyNutJob *job = new LazyNutJob;
        job->logMode |= ECHO_INTERPRETER;
        job->cmdList = QStringList({
                            QString("create %1 %2").arg(dataframeType).arg(dfName),
                            QString("%1 %2 %3").arg(dfName).arg(loadCmd).arg(fileName)});
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job
                << SessionManager::instance()->recentlyCreatedJob()
                << SessionManager::instance()->recentlyModifiedJob();
        QMap<QString, QVariant> jobData;
        jobData.insert("name", dfName);
        jobData.insert("isBackup", false);
        jobs.last()->data = jobData;
        jobs.last()->appendEndOfJobReceiver(this, SLOT(addItem()));
        SessionManager::instance()->submitJobs(jobs);
    }
}

void DataframeViewer::save()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Save as CSV file"),
                        lastSaveDir.isEmpty() ? defaultSaveDir : lastOpenDir,
                        "CSV (*.csv)");
    if (!fileName.isEmpty())
    {
        LazyNutJob *job = new LazyNutJob;
        job->logMode |= ECHO_INTERPRETER;
        job->cmdList = QStringList({QString("%1 save_csv %2").arg(ui->currentItemName()).arg(fileName)});
        SessionManager::instance()->submitJobs(job);
        lastSaveDir = QFileInfo(fileName).path();
    }
}

void DataframeViewer::copy()
{
    // this is an illegal approach -- get R to copy the df to the clipboard
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList = QStringList({QString("R << write.table(eN[\"%1\"], \"clipboard\", sep=\"\t\", row.names=FALSE)")
                                .arg(ui->currentItemName())});
    SessionManager::instance()->submitJobs(job);
}


void DataframeViewer::copyDataframe()
{
    QString originalDf = ui->currentItemName();
    if (originalDf.isEmpty())
        return;
    QString copyDf = SessionManager::instance()->makeValidObjectName(originalDf);
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList << QString("%1 copy %2").arg(originalDf).arg(copyDf);
    QMap<QString, QVariant> jobData;
    jobData.insert("name", copyDf);
    jobData.insert("isBackup", true);
    if (dispatcher)
    {
//        jobData.insert("trialRunInfo", dispatcher->infoVariantList(originalDf));
        SessionManager::instance()->copyTrialRunInfo(originalDf, copyDf);
        setPrettyHeadersForTrial(dispatcher->trial(originalDf), copyDf);
    }
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->recentlyCreatedJob();
    jobs.last()->data = jobData;
    jobs.last()->appendEndOfJobReceiver(this, SLOT(addItem()));
    SessionManager::instance()->submitJobs(jobs);

}

void DataframeViewer::addRequestedItem(QString name, bool isBackup)
{
    if (requestedDataframeViews.contains(name))
        addItem(name, isBackup);
    requestedDataframeViews.removeAll(name);
}


void DataframeViewer::destroyItem_impl(QString name)
{
        delete modelMap.value(name, nullptr);
        modelMap.remove(name);
        if (prettyHeadersModelMap.contains(name))
            delete prettyHeadersModelMap.value(name);
}

void DataframeViewer::enableActions(bool enable)
{
    DataViewer::enableActions(enable);
    findAct->setEnabled(enable);
    copyDFAct->setEnabled(enable);
    plotButton->setEnabled(enable);
    dataframeViewButton->setEnabled(enable);
//    QString subtype;
//    QDomDocument *domDoc = dataframeDescriptionFilter->getDomDoc(ui->currentItemName());
//    if (domDoc)
//        subtype = XMLelement(*domDoc)["subtype"]();
//    ui->settingsAct->setEnabled(enabled && (subtype == "dataframe_view"));
}

void DataframeViewer::setCurrentItem(QString name)
{
    DataViewer::setCurrentItem(name);
    if (isLazy())
    {
        dataframeFilter->setName(name);
//        dataframeDescriptionFilter->setName(name);
    }
}


void DataframeViewer::updateDataframe(QDomDocument *domDoc, QString name)
{
    if (!modelMap.contains(name))
    {
        eNerror << QString("attempt to update non-existing dataframe %1").arg(name);
        return;
    }
    DataFrameModel *dfModel = new DataFrameModel(domDoc, this);
    if (parametersTable())
        connect(dfModel, SIGNAL(newParamValueSig(QString,QString)),
                this, SLOT(setParameter(QString,QString)));

    dfModel->setName(name); // needed?
    PrettyHeadersModel *prettyHeadersModel = nullptr;
    if (prettyHeadersModelMap.contains(name))
    {
        prettyHeadersModel = prettyHeadersModelMap.value(name);
        prettyHeadersModel->setSourceModel(dfModel);
    }
    bool isNewModel = (modelMap.value(name) == nullptr);
    // needs to be changed for splitters
    QTableView *view = qobject_cast<QTableView*>(ui->view(name));
    if (isNewModel)
    {
//        view = new QTableView(this);
        if (dragDropColumns())
        {
             DataFrameHeader* dragDropHeader = new DataFrameHeader(view);
             view->setHorizontalHeader(dragDropHeader);
             dragDropHeader->setTableName(name);
             connect(dragDropHeader, SIGNAL(columnDropped(QString)),
                     MainWindow::instance()->trialWidget, SLOT(showSetLabel(QString)));
             connect(dragDropHeader, SIGNAL(restoreComboBoxText()),
                     MainWindow::instance()->trialWidget, SLOT(restoreComboBoxText()));
        }
        if (parametersTable())
            view->setEditTriggers(QAbstractItemView::AllEditTriggers);
        else
            view->setEditTriggers(QAbstractItemView::NoEditTriggers);
//        viewMap.insert(name, view);
//        ui->addItem(name, view);
    }
    else
    {
        DataFrameModel *oldDFmodel = modelMap.value(name);
        QItemSelectionModel *m = view->selectionModel();
        delete oldDFmodel;
        delete m;
    }
    modelMap[name] = dfModel;
    if (prettyHeadersModel)
        view->setModel(prettyHeadersModel);
    else
        view->setModel(dfModel);
    view->verticalHeader()->hide();
    view->horizontalHeader()->show();
    view->resizeColumnsToContents();
}

void DataframeViewer::showFindDialog()
{
    if (ui->currentItemName().isEmpty())
        return;
    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
}

void DataframeViewer::findForward(const QString &str, QFlags<QTextDocument::FindFlag> flags)
{
    Q_UNUSED(flags)
    QFlags<Qt::MatchFlag> flag;
//    if (flags |= QTextDocument::FindCaseSensitively)
//        flag = Qt::MatchCaseSensitive;
//    else
        flag = Qt::MatchExactly;
    QVariant qv(str);
    QString name = ui->currentItemName();
    if (name.isEmpty())
        return;
    QTableView *view = qobject_cast<QTableView*>(ui->view(name));
    DataFrameModel*dfModel = modelMap.value(name);

    // first try searching in the current column
    int row = view->currentIndex().row();
    int col = view->currentIndex().column();
    if (row<0)
        row=0;
    if (col<0)
        col=0;

    QModelIndexList hits = dfModel->match(dfModel->index(row, col),
                            Qt::DisplayRole,qv,1,flag);
    if (hits.size() == 0)
    {
        //now try a more systematic approach
        for (int i=0;i<dfModel->columnCount();i++)
        {
            hits = dfModel->match(dfModel->index(0, i),
                                Qt::DisplayRole,qv);
            if (hits.size() > 0)
                break;
        }
    }

    if (hits.size() > 0)
    {
        view->setCurrentIndex(hits.first());
//        findDialog->hide();
    }
    else
        QMessageBox::warning(this, "Find",QString("The text was not found"));
//        findDialog->hide();

}

void DataframeViewer::setParameter(QString name, QString key_val)
{
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList << QString("%1 set %2").arg(name).arg(key_val);
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->recentlyModifiedJob();
    SessionManager::instance()->submitJobs(jobs);
}

void DataframeViewer::sendNewPlotRequest()
{
    QAction *plotAct = qobject_cast<QAction *>(sender());
    if (!plotAct)
        eNerror << "invalid sender action for this method";
    else
    {
        QString plotType = plotAct->text();
        QMap<QString,QString> settings;
        settings["df"] = ui->currentItemName();
        QString plotName = SessionManager::instance()->makeValidObjectName(ui->currentItemName());
//        QList<QSharedPointer<QDomDocument> > info = dispatcher ? SessionManager::instance()->trialRunInfo(ui->currentItemName()) : QList<QSharedPointer<QDomDocument> >();
        emit createDataViewRequested(plotName, "rplot", plotType, settings);
    }
}

void DataframeViewer::sendNewDataframeViewRequest()
{
    QAction *dataframeViewAct = qobject_cast<QAction *>(sender());
    if (!dataframeViewAct)
        eNerror << "invalid sender action for this method";
    else
    {
        QString dataframeViewScript = dataframeViewAct->text();
        QMap<QString,QString> settings;
        settings["df"] = ui->currentItemName();
        QString dataframeViewName = SessionManager::instance()->makeValidObjectName(ui->currentItemName());
//        QList<QSharedPointer<QDomDocument> > info = dispatcher ? SessionManager::instance()->trialRunInfo(ui->currentItemName()) : QList<QSharedPointer<QDomDocument> >();
        requestedDataframeViews.append(dataframeViewName);
//        emit newDataframeViewRequested(dataframeViewName, dataframeViewScript, settings);
        emit createDataViewRequested(dataframeViewName, "dataframe_view", dataframeViewScript, settings);
    }
}

void DataframeViewer::addItem_impl(QString name)
{
    modelMap.insert(name, nullptr);
}

QWidget *DataframeViewer::makeView(QString name)
{
    Q_UNUSED(name);
    return new QTableView(this);
}

void DataframeViewer::addNameToFilter(QString name)
{
    if (SessionManager::instance()->exists(name))
    {
        dataframeFilter->addName(name);
    }
}

void DataframeViewer::removeNameFromFilter(QString name)
{
    dataframeFilter->removeName(name);
}

void DataframeViewer::setNameInFilter(QString name)
{
    dataframeFilter->setName(name);
}

void DataframeViewer::addExtraActions()
{
    findAct = new QAction(QIcon(":/images/magnifying-glass-2x.png"), tr("&Find"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setToolTip(tr("Find text in this table"));
    findAct->setVisible(true);
    findAct->setEnabled(false);
    ui->editToolBar->addAction(findAct);
    connect(findAct, SIGNAL(triggered()), this, SLOT(showFindDialog()));

    copyDFAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy to new dataframe"), this);
    copyDFAct->setStatusTip(tr("Copy contents to a new dataframe"));
    copyDFAct->setVisible(true);
    copyDFAct->setEnabled(false);
    ui->editToolBar->addAction(copyDFAct);
    connect(copyDFAct, SIGNAL(triggered()), this, SLOT(copyDataframe()));

//    dataframeMergeAct = new QAction(QIcon(":/images/Merge_Icon.png"), tr("&Merge two dataframes"), this);
//    dataframeMergeAct->setStatusTip(tr("Merge two dataframes"));
//    dataframeMergeAct->setVisible(true);
//    dataframeMergeAct->setEnabled(false);
//    ui->editToolBar->addAction(dataframeMergeAct);
//    connect(dataframeMergeAct, SIGNAL(triggered()), this, SLOT(dataframeMerge()));



    plotButton = new QToolButton(this);
    plotButton->setIcon(QIcon(":/images/barchart2.png"));
    plotButton->setVisible(true);
    plotButton->setEnabled(false);
    plotButton->setPopupMode(QToolButton::InstantPopup);
    QMenu *plotMenu = new QMenu(plotButton);
    // temporary: show all available R scripts
    QDir rPlotsDir(SessionManager::instance()->defaultLocation("rPlotsDir"));
    rPlotsDir.setNameFilters(QStringList({"*.R"}));
    foreach(QString plotType, rPlotsDir.entryList())
    {
        QAction *plotAct = new QAction(plotType, this);
        connect(plotAct, SIGNAL(triggered()), this, SLOT(sendNewPlotRequest()));
        connect(plotAct, SIGNAL(triggered()), MainWindow::instance(), SLOT(showDataViewSettings()));
        plotMenu->addAction(plotAct);
    }
    plotButton->setMenu(plotMenu);
    ui->editToolBar->addWidget(plotButton);

    dataframeViewButton = new QToolButton(this);
    dataframeViewButton->setIcon(QIcon(":/images/formula.png"));
    dataframeViewButton->setVisible(true);
    dataframeViewButton->setEnabled(false);
    dataframeViewButton->setPopupMode(QToolButton::InstantPopup);
    QMenu *dataframeViewMenu = new QMenu(dataframeViewButton);
    QDir rDataframeViewsDir(SessionManager::instance()->defaultLocation("rDataframeViewsDir"));
    rDataframeViewsDir.setNameFilters(QStringList({"*.R"}));
    foreach(QString dataframeViewScript, rDataframeViewsDir.entryList())
    {
        QAction *dataframeViewAct = new QAction(dataframeViewScript, this);
        connect(dataframeViewAct, SIGNAL(triggered()), this, SLOT(sendNewDataframeViewRequest()));
        connect(dataframeViewAct, SIGNAL(triggered()), MainWindow::instance(), SLOT(showDataViewSettings()));
        dataframeViewMenu->addAction(dataframeViewAct);
    }
    dataframeViewButton->setMenu(dataframeViewMenu);
    ui->editToolBar->addWidget(dataframeViewButton);

}

void DataframeViewer::setPrettyHeadersForTrial(QString trial, QString df)
{
    PrettyHeadersModel *prettyHeadersModel = new PrettyHeadersModel(this);
/*    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal, "event_pattern", "");
    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal,"\\(", "");
    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal,"\\)", "");
    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal,trial, "");
    prettyHeadersModelMap.insert(df, prettyHeadersModel);
*/
}

void DataframeViewer::dispatch()
{
    DataViewer::dispatch();
}

