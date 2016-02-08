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
#include "settingsformdialog.h"


#include <QSettings>
#include <QFileDialog>
#include <QTableView>

DataframeViewer::DataframeViewer(Ui_DataViewer *ui, QWidget *parent)
    : DataViewer(ui, parent), m_dragDropColumns(false), m_stimulusSet(false), m_parametersTable(false)
{
    dataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    dataframeUpdater = new ObjectUpdater(this);
    dataframeUpdater->setCommand("get");
    dataframeUpdater->setProxyModel(dataframeFilter);
    connect(dataframeUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)),
            this, SLOT(updateDataframe(QDomDocument*,QString)));
    destroyedObjectsFilter->setType("dataframe");
    findDialog = new FindDialog(this);
    findDialog->hideExtendedOptions();
    connect(findDialog, SIGNAL(findForward(QString, QFlags<QTextDocument::FindFlag>)),
            this, SLOT(findForward(QString, QFlags<QTextDocument::FindFlag>)));

    // because DataframeViewer::enableActions won't be called by te ctor, only DataViewer::enableActions
    ui->findAct->setVisible(true);
    ui->findAct->setEnabled(false);
    connect(ui->findAct, SIGNAL(triggered()), this, SLOT(showFindDialog()));

    ui->copyDFAct->setVisible(true);
    ui->copyDFAct->setEnabled(false);
    connect(ui->copyDFAct, SIGNAL(triggered()), this, SLOT(copyDataframe()));

    ui->dataframeMergeAct->setVisible(true);
    ui->dataframeMergeAct->setEnabled(false);
    connect(ui->dataframeMergeAct, SIGNAL(triggered()), this, SLOT(dataframeMerge()));
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

        fileName = QDir(MainWindow::instance()->easyNetDataHome).relativeFilePath(fileName);
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
        jobData.insert("setCurrent", true);
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
    jobData.insert("dfName", copyDf);
    jobData.insert("setCurrent", true);
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->recentlyCreatedJob();
    jobs.last()->data = jobData;
    jobs.last()->appendEndOfJobReceiver(this, SLOT(addItem()));
    SessionManager::instance()->submitJobs(jobs);
    if (dispatcher)
    {
        dispatcher->copyTrialRunInfo(originalDf, copyDf);
        setPrettyHeadersForTrial(dispatcher->trial(originalDf), copyDf);
    }
}

void DataframeViewer::dataframeMerge()
{
    // load XML
    QDomDocument* domDoc = new QDomDocument;
    QSettings settings("QtEasyNet", "nmConsole");
    QString easyNetHome = settings.value("easyNetHome","../..").toString();
    QFile file(QString("%1/XML_files/dataframe_merge.xml").arg(easyNetHome));
    if (!file.open(QIODevice::ReadOnly) || !domDoc->setContent(&file))
    {
        delete domDoc;
        file.close();
        return;
    }
    file.close();
    // setup form
    SettingsForm *form = new SettingsForm(domDoc, this);
    form->setUseRFormat(false);
    QMap<QString, QString> preFilledSettings;
    preFilledSettings["x"] = ui->currentItemName();
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
        QMap<QString, QVariant> jobData;
        jobData.insert("dfName", dfName);
        jobData.insert("setCurrent", true);
        jobs.last()->data = jobData;
        jobs.last()->appendEndOfJobReceiver(this, SLOT(addItem()));
        jobs.last()->appendEndOfJobReceiver(this, SLOT(refreshInfo()));
        SessionManager::instance()->submitJobs(jobs);
        if (dispatcher)
        {
            // only one will succeed
            dispatcher->copyTrialRunInfo(x, dfName);
            dispatcher->copyTrialRunInfo(y, dfName);
        }
        SessionManager::instance()->addDataframeMerge(x, dfName);
        SessionManager::instance()->addDataframeMerge(y, dfName);
    });
    dialog.exec();
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
    ui->findAct->setEnabled(enable);
    ui->copyDFAct->setEnabled(enable);
    ui->dataframeMergeAct->setEnabled(enable);
}

void DataframeViewer::updateCurrentItem(QString name)
{
    DataViewer::updateCurrentItem(name);
    if (isLazy())
        dataframeFilter->setName(name);
}



//void DataframeViewer::addItem(QString name, bool setCurrent)
//{
//    if (name.isEmpty())
//    {
//        QVariant v = SessionManager::instance()->getDataFromJob(sender(), "dfName");
//        if (!v.canConvert<QString>())
//        {
//            eNerror << "cannot retrieve a valid string from dfName key in sender LazyNut job";
//            return;
//        }
//        name = v.value<QString>();
//        v = SessionManager::instance()->getDataFromJob(sender(), "setCurrent");
//        if (v.canConvert<bool>())
//            setCurrent = v.value<bool>();
//    }
//    if (name.isEmpty())
//    {
//        eNerror << "name is empty";
//    }
//    else if (!SessionManager::instance()->exists(name))
//    {
//        eNerror << QString("attempt to add a non-existing dataframe %1").arg(name);
//    }
//    else if (modelMap.contains(name))
//    {
////        eNerror << QString("attempt to create an already existing model for dataframe %1").arg(name);
//        if (setCurrent)
//            ui->setCurrentItem(name);
//    }
//    else
//    {
//        modelMap.insert(name, nullptr);
//        QTableView *view = new QTableView(this);
//        viewMap.insert(name, view);
//        ui->addItem(name, view);
//        if (setCurrent)
//            ui->setCurrentItem(name);
//        if (!isLazy())
//            dataframeFilter->addName(name);
//    }
//}

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
        dataframeFilter->addName(name);
}

void DataframeViewer::removeNameFromFilter(QString name)
{
    dataframeFilter->removeName(name);
}

void DataframeViewer::setNameInFilter(QString name)
{
    dataframeFilter->setName(name);
}

void DataframeViewer::setPrettyHeadersForTrial(QString trial, QString df)
{
    PrettyHeadersModel *prettyHeadersModel = new PrettyHeadersModel(this);
    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal, "event_pattern", "");
    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal,"\\(", "");
    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal,"\\)", "");
    prettyHeadersModel->addHeaderReplaceRules(Qt::Horizontal,trial, "");
    prettyHeadersModelMap.insert(df, prettyHeadersModel);
}

void DataframeViewer::dispatch()
{
    DataViewer::dispatch();
}

