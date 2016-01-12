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



#include <QSettings>
#include <QFileDialog>
#include <QTableView>

DataframeViewer::DataframeViewer(QWidget *parent)
    : DataViewer(parent), lastSaveDir("")
{
    dataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    dataframeUpdater = new ObjectUpdater(this);
    dataframeUpdater->setCommand("get");
    dataframeUpdater->setProxyModel(dataframeFilter);
    connect(dataframeUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)),
            this, SLOT(updateDataframe(QDomDocument*,QString)));
}

DataframeViewer::~DataframeViewer()
{

}

void DataframeViewer::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Import dataframe"),
                                                    lastOpenDir.isEmpty() ? MainWindow::instance()->stimDir : lastOpenDir,
                                                    tr("Database Files (*.eNd);;Text files (*.csv);;All files (*.*)"));
    if (!fileName.isEmpty())
    {
        // create db
        QFileInfo fi(fileName);
        QString dfName = SessionManager::instance()->makeValidObjectName(fi.completeBaseName());
        lastOpenDir = fi.path();

        fileName = QDir(MainWindow::instance()->easyNetDataHome).relativeFilePath(fileName);
        QString loadCmd = fi.suffix() == "csv" ? "load_csv" : "load";
        LazyNutJob *job = new LazyNutJob;
        job->logMode |= ECHO_INTERPRETER;
        job->cmdList = QStringList({
                            QString("create dataframe %1").arg(dfName),
                            QString("%1 %2 %3").arg(dfName).arg(loadCmd).arg(fileName)});
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job
                << SessionManager::instance()->recentlyCreatedJob();
        QMap<QString, QVariant> jobData;
        jobData.insert("dfName", dfName);
        jobs.last()->data = jobData;
        jobs.last()->appendEndOfJobReceiver(this, SLOT(addDataframe()));
        SessionManager::instance()->submitJobs(jobs);
    }
}

void DataframeViewer::save()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Save as CSV file"),
                        lastSaveDir.isEmpty() ? QString("%1/Databases").arg(MainWindow::instance()->easyNetDataHome) : lastOpenDir,
                        "CSV (*.csv)");
    if (!fileName.isEmpty())
    {
        LazyNutJob *job = new LazyNutJob;
        job->logMode |= ECHO_INTERPRETER;
        job->cmdList = QStringList({QString("%1 save_csv %2").arg(ui->currentItem()).arg(fileName)});
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
                                .arg(ui->currentItem())});
    SessionManager::instance()->submitJobs(job);
}

void DataframeViewer::removeItem(QString name)
{
    if (!modelMap.contains(name))
    {
        eNerror << QString("attemt to delete non-existing dataframe %1").arg(name);
    }
    else if (!modelMap.value(name))
    {
        eNerror << QString("dataframe %1 does not have a DataFrameModel").arg(name);
    }
    else if (!viewsMap.values(name).at(0)) // to be changed for splitters
    {
        eNerror << QString("dataframe %1 does not have a view").arg(name);
    }
    else
    {
        ui->removeItem(name);
        delete modelMap.value(name);
        delete viewsMap.values(name).at(0);
        if (prettyHeadersModelMap.contains(name))
            delete prettyHeadersModelMap.value(name);
        // don't destroy default dataframes
        if (!name.contains(QRegExp("[()]"))) // their names contain brackets
            SessionManager::instance()->destroyObject(name);
    }
}

void DataframeViewer::updateCurrentItem(QString name)
{
    if (name.isEmpty())
        enableActions(false);
    else
        enableActions(true);
}

void DataframeViewer::addDataframe()
{
    QVariant v = SessionManager::instance()->getDataFromJob(sender(), "dfName");
    if (!v.canConvert<QString>())
    {
        eNerror << "cannot retrieve a valid string from dfName key in sender LazyNut job";
        return;
    }
    QString dfName = v.value<QString>();
    if (dfName.isEmpty())
    {
        eNerror << "string from dfName key in sender LazyNut job is empty";
    }
    else if (!SessionManager::instance()->exists(dfName))
    {
        eNerror << QString("attempt to add a non-existing dataframe %1").arg(dfName);
    }
    else if (modelMap.contains(dfName))
    {
        eNerror << QString("attempt to create an already existing model for dataframe %!").arg(dfName);
    }
    else
    {
        modelMap.insert(dfName, nullptr);
        dataframeFilter->addName(dfName);
    }
}

void DataframeViewer::updateDataframe(QDomDocument *domDoc, QString name)
{
    if (!modelMap.contains(name))
    {
        eNerror << QString("attempt to update non-existing dataframe %1").arg(name);
    }
    DataFrameModel *dfModel = new DataFrameModel(domDoc, this);
    dfModel->setName(name); // needed?
    PrettyHeadersModel *prettyHeadersModel = nullptr;
    if (prettyHeadersModelMap.contains(name))
    {
        prettyHeadersModel = prettyHeadersModelMap.value(name);
        prettyHeadersModel->setSourceModel(dfModel);
    }
    bool isNewModel = (modelMap.value(name) == nullptr);
    // needs to be changed for splitters
    QTableView *view;
    if (isNewModel)
    {
        view = new QTableView(this);
        viewsMap.insert(name, view);
        ui->addItem(name, view);
    }
    else
    {
        DataFrameModel *oldDFmodel = modelMap.value(name);
        view = viewsMap.values(name).at(0); // take the first and only one for the moment, to be changed for splitters
        QItemSelectionModel *m = view->selectionModel();
        delete oldDFmodel;
        delete m;
    }
    modelMap[name] = dFmodel;
    view->setModel(prettyHeadersModel ? prettyHeadersModel : dfModel);
    view->verticalHeader()->hide();
    view->resizeColumnsToContents();
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

