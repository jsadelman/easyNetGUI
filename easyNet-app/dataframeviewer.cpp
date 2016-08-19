#include "dataframeviewer.h"
#include "easyNetMainWindow.h"
#include "lazynutjob.h"
#include "enumclasses.h"
#include "sessionmanager.h"
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
#include "parametersproxymodel.h"
#include "dataframeviewerdispatcher.h"


#include <QSettings>
#include <QFileDialog>
#include <QTableView>

Q_DECLARE_METATYPE(QSharedPointer<QDomDocument> )


DataframeViewer::DataframeViewer(Ui_DataViewer *ui, QWidget *parent)
    : DataViewer(ui, parent), m_dragDropColumns(false), m_stimulusSet(false),
      maxRows(80), maxCols(20), maxFirstDisplayCells(3000), maxDisplayCells(10000),
      m_showInMainViewer(true)
{
    dataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    dataframeUpdater = new ObjectUpdater(this);
    dataframeUpdater->setCommand(QString("get 1-%1 1-%2").arg(maxRows).arg(maxCols));
    dataframeUpdater->setProxyModel(dataframeFilter);
    connect(dataframeUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)),
            this, SLOT(updateDataframe(QDomDocument*,QString)));
    connect(dataframeUpdater, &ObjectUpdater::objectUpdated, [=](QDomDocument*,QString name)
    {
        if (dispatcher)
            dispatcher->updateInfo(name);
    });

    findDialog = new FindDialog(this);
    findDialog->hideExtendedOptions();
    connect(findDialog, SIGNAL(findForward(QString, QFlags<QTextDocument::FindFlag>)),
            this, SLOT(findForward(QString, QFlags<QTextDocument::FindFlag>)));
    requestedDataframeViews.clear();

    addExtraActions();
}

void DataframeViewer::open()
{
    QString dataframeType = stimulusSet() ? "stimulus_set" : "dataframe";
    qDebug() << "Entered open() in DataframeViewer" << this << dataframeType;
    QString caption = stimulusSet() ? tr("Load stimulus_set") : tr("Import dataframe");
    QString filter = stimulusSet() ? tr("Stimulus set files (*.eNd);;CSV files (*.csv);;All files (*.*)") : tr("CSV files (*.csv);;Database files (*.eNd);;All files (*.*)");
    QString fileName = QFileDialog::getOpenFileName(this,caption,
                                                    lastOpenDir.isEmpty() ? defaultOpenDir() : lastOpenDir,
                                                    filter);
    if (!fileName.isEmpty())
    {
        // create db
        QFileInfo fi(fileName);
        QString dfName = SessionManager::instance()->makeValidObjectName(fi.completeBaseName());
        lastOpenDir = fi.path();

        fileName = QDir(SessionManager::instance()->easyNetDataHome()).relativeFilePath(fileName);
        QString loadCmd = fi.suffix() == "csv" ? "load_csv" : "load";
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = QStringList({
                            QString("create %1 %2").arg(dataframeType).arg(dfName),
                            QString("%1 add_hint show %2").arg(dfName).arg(m_showInMainViewer ? "1" : "0"),
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
    // this is an illegal approach -- get R to copy the df to file
    QDomDocument *description = SessionManager::instance()->description(ui->currentItemName());
    QString prettyName = description ? XMLelement(*description)["pretty name"]() : ui->currentItemName();
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Save as CSV file"),
                        QString("%1/%2").arg(lastSaveDir.isEmpty() ? defaultSaveDir() : lastSaveDir).arg(prettyName),
                        "CSV (*.csv)");
    if (!fileName.isEmpty())
    {
        fileName = QDir(SessionManager::instance()->easyNetDataHome()).relativeFilePath(fileName);
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = QStringList({QString("%1 save_csv %2").arg(ui->currentItemName()).arg(fileName)});
        SessionManager::instance()->submitJobs(job);
        lastSaveDir = QFileInfo(fileName).path();
    }
}

void DataframeViewer::copy()
{
    if (!dataframeExceedsCellLimit(ui->currentItemName(), maxDisplayCells))
        doCopy();
    else
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        if (!SessionManager::instance()->descriptionCache->getDomDoc(ui->currentItemName()))
        {
            // this should not happen
            msgBox.setText("Unknown size dataframe");
            msgBox.setInformativeText(QString("The size of the requested dataframe cannot be determined.\n"
                                      "Do you want to copy it to clipboard anyway or rather save it to file?").arg(maxDisplayCells));
        }
        else
        {
            msgBox.setText("Large dataframe");
            msgBox.setInformativeText(QString("The requested dataframe contains more than %1 cells.\n"
                                              "Do you want to copy it to clipboard anyway or rather save it to file?").arg(maxDisplayCells));
        }
        msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Cancel);
        QPushButton *fullCopyButton = msgBox.addButton(tr("Full copy"), QMessageBox::RejectRole);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Cancel)
            return;
        else if (ret == QMessageBox::Save)
            save();
        else if (msgBox.clickedButton() == fullCopyButton)
            doCopy();
        else
            return;
    }
}


void DataframeViewer::copyDataframe()
{
    QString originalDf = ui->currentItemName();
    if (originalDf.isEmpty())
        return;
    QString copyDf = SessionManager::instance()->makeValidObjectName(originalDf);
    LazyNutJob *job = new LazyNutJob;
    job->cmdList << QString("%1 copy %2").arg(originalDf).arg(copyDf);
    QMap<QString, QVariant> jobData;
    jobData.insert("name", copyDf);
    jobData.insert("isBackup", true);
    if (dispatcher)
    {
        SessionManager::instance()->copyTrialRunInfo(originalDf, copyDf);
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
}

void DataframeViewer::enableActions(bool enable)
{
    DataViewer::enableActions(enable);
    getAllAct->setEnabled(enable && partiallyLoaded(ui->currentItemName()));
    findAct->setEnabled(enable);
    copyDFAct->setEnabled(enable);
    plotButton->setEnabled(enable);
    dataframeViewButton->setEnabled(enable);
    if (dispatcher)
        dispatcher->enableActions(enable);

}

bool DataframeViewer::setCurrentItem(QString name)
{
    if (!DataViewer::setCurrentItem(name))
        return false;
    if (isLazy())
    {
        dataframeFilter->setName(name);
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
    dfModel->setName(name);
    bool isNew = !modelMap.value(name, nullptr);
    QTableView *view = qobject_cast<QTableView*>(ui->view(name));
    if (isNew && view)
    {
        if (dragDropColumns())
        {
            DataFrameHeader* dragDropHeader = new DataFrameHeader(view);
            view->setHorizontalHeader(dragDropHeader);
            dragDropHeader->setTableName(name);
            connect(dragDropHeader, SIGNAL(columnDropped(QString)),
                    MainWindow::instance()->trialWidget, SLOT(showSetLabel(QString)));
        }
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    else if (!isNew)
    {
        DataFrameModel *oldDFmodel = modelMap.value(name);
        QItemSelectionModel *m = view ? view->selectionModel() : nullptr;
        delete oldDFmodel;
        delete m;
    }
    modelMap[name] = dfModel;
    limitedGet(name, maxFirstDisplayCells);
    if (view)
    {
        view->setModel(modelMap[name]);
        view->verticalHeader()->show();
        view->horizontalHeader()->show();
        view->resizeColumnsToContents();
    }
    enableActions(true);
}

void DataframeViewer::askGetEntireDataframe()
{
    if (!dataframeExceedsCellLimit(ui->currentItemName(), maxDisplayCells))
        getEntireDataframe();
    else
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        if (!SessionManager::instance()->descriptionCache->getDomDoc(ui->currentItemName()))
        {
            // this should not happen
            msgBox.setText("Unknown size dataframe");
            msgBox.setInformativeText(QString("The size of the requested dataframe cannot be determined.\n"
                                      "Do you want to load it entirely or rather load up to %1 cells?").arg(maxDisplayCells));
        }
        else
        {
            msgBox.setText("Large dataframe");
            msgBox.setInformativeText(QString("The requested dataframe contains more than %1 cells.\n"
                                              "Do you want to load it anyway or rather load up to %1 cells?").arg(maxDisplayCells));
        }
        msgBox.setStandardButtons(QMessageBox::Cancel);
        QPushButton *limitedLoadButton = msgBox.addButton(tr("Limited load"), QMessageBox::AcceptRole);
        QPushButton *fullLoadButton = msgBox.addButton(tr("Full load"), QMessageBox::RejectRole);
        msgBox.setDefaultButton(limitedLoadButton);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Cancel)
            return;
        if (msgBox.clickedButton() == limitedLoadButton)
            limitedGet(ui->currentItemName(), maxDisplayCells);
        else if (msgBox.clickedButton() == fullLoadButton)
            getEntireDataframe();
        else
            return;
    }
}

void DataframeViewer::getEntireDataframe()
{
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("xml %1 get").arg(ui->currentItemName())});
    job->setAnswerReceiver(SessionManager::instance()->dataframeCache, SLOT(setDomDocAndValidCache(QDomDocument*, QString)), AnswerFormatterType::XML);
    SessionManager::instance()->submitJobs(job);
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
    job->cmdList << QString("%1 set %2").arg(name).arg(key_val);
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->recentlyModifiedJob();
    SessionManager::instance()->submitJobs(jobs);
}

void DataframeViewer::buildPlotMenu()
{
    buildRScriptMenu(plotMenu, "rPlotsDir", "rplot");
}

void DataframeViewer::buildDataframeViewMenu()
{
    buildRScriptMenu(dataframeViewMenu, "rDataframeViewsDir", "dataframe_view");
}

void DataframeViewer::buildRScriptMenu(QMenu *menu, QString defaultLocation, QString subtype)
{
    foreach(QAction *action, menu->actions())
        delete action;
    QDir rScriptsDir(SessionManager::instance()->defaultLocation(defaultLocation));
    rScriptsDir.setNameFilters(QStringList({"*.R"}));
    foreach(QString script, rScriptsDir.entryList())
    {
        QAction *action = new QAction(script, this);
        action->setData(subtype);
        connect(action, SIGNAL(triggered()), this, SLOT(sendNewDataViewRequest()));
        menu->addAction(action);
    }
}

QString DataframeViewer::sendNewDataViewRequest(QString dataViewScript, QString subtype, bool setForm, bool popUpSettings)
{
    if (dataViewScript.isEmpty() || subtype.isEmpty())
    {
        QAction *action = qobject_cast<QAction*>(sender());
        if (!action)
        {
            eNerror << "invalid QAction argument";
            return QString();
        }
        if (action->text().isEmpty())
        {
            eNerror << "QAction does not contain text, while it should contain an R script name";
            return QString();
        }
        dataViewScript = action->text();
        subtype = action->data().toString();
    }
    if (!(subtype == "dataframe_view" || subtype == "rplot"))
    {
        eNerror << "invalid subtype:" << subtype;
        return QString();
    }

    QString suffix = dataViewScript;
    suffix.remove(QRegExp("\\.R$"));
    QMap<QString,QString> settings;
    settings["df"] = ui->currentItemName();
    QString prettyName = subtype == "dataframe_view" ? SessionManager::instance()->nextPrettyName(itemPrettyName()) : "";
    QString dataViewName = SessionManager::instance()->makeValidObjectName(QString("%1.%2.1").arg(ui->currentItemName()).arg(suffix));
    LazyNutJob *job = SessionManager::instance()->createDataViewJob(dataViewName, prettyName, subtype, dataViewScript, settings);
    if (setForm && !popUpSettings)
        job->cmdList << QString("%1 add_hint show 1").arg(dataViewName);

    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->updateObjectCacheJobs();
    QMap<QString, QVariant> jobData;
    jobData.insert("name", dataViewName);
    jobs.last()->data = jobData;
    if (setForm)
    {
        jobs.last()->appendEndOfJobReceiver(MainWindow::instance(), SLOT(createNewForm()));
//        jobs.last()->appendEndOfJobReceiver(MainWindow::instance(), SLOT(showResultsViewer()));
    }
    if (popUpSettings)
        jobs.last()->appendEndOfJobReceiver(MainWindow::instance(), SLOT(showDataViewSettings()));

    SessionManager::instance()->submitJobs(jobs);
    return dataViewName;
}

void DataframeViewer::merge()
{
    QString dataViewScript = "merge.R";
    QFile dataViewScriptFile(QString("%1/%2").arg(SessionManager::instance()->defaultLocation("rDataframeViewsDir")).arg(dataViewScript));
    if (!dataViewScriptFile.exists())
    {
        eNerror << dataViewScript << "not found";
        return;
    }
    QString mergeName = sendNewDataViewRequest(dataViewScript, "dataframe_view", true, false);
    if (!mergeName.isEmpty())
        setViewState(mergeName, ViewState_Fresh);
}

void DataframeViewer::addItem_impl(QString name)
{
    modelMap.insert(name, nullptr);
}

void DataframeViewer::setFirstViewState(QString name)
{
    QString subtype = SessionManager::instance()->descriptionCache->subtype(name);
    if (subtype == "dataframe_for_activities" || subtype == "dataframe_for_trials" || subtype == "dataframe_view")
    {
        if (viewState(name) != ViewState_Fresh)
            setViewState(name, ViewState_Stale);
    }
    else
        setViewState(name, ViewState_Static);
}

QWidget *DataframeViewer::makeView(QString name)
{
    Q_UNUSED(name);
    return new QTableView(this);
}

void DataframeViewer::addNameToFilter(QString name)
{
    SessionManager::instance()->dataframeCache->create(name); // for hidden items
    dataframeFilter->addName(name);
}

void DataframeViewer::removeNameFromFilter(QString name)
{
    dataframeFilter->removeName(name);
}

void DataframeViewer::setNameInFilter(QString name)
{
    SessionManager::instance()->dataframeCache->create(name); // for hidden items
    dataframeFilter->setName(name);
}

void DataframeViewer::addExtraActions()
{
    getAllAct = new QAction(QIcon(":/images/download.png"), "get entire table", this);
    getAllAct->setToolTip("Get the entire dataframe");
    getAllAct->setVisible(true);
    getAllAct->setEnabled(false);
    ui->editToolBar[this]->addAction(getAllAct);
    connect(getAllAct, SIGNAL(triggered()), this, SLOT(askGetEntireDataframe()));

    findAct = new QAction(QIcon(":/images/magnifying-glass-2x.png"), tr("&Find"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setToolTip(tr("Find text in this table"));
    findAct->setVisible(true);
    findAct->setEnabled(false);
    ui->editToolBar[this]->addAction(findAct);
    connect(findAct, SIGNAL(triggered()), this, SLOT(showFindDialog()));

    copyDFAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy to new dataframe"), this);
    copyDFAct->setStatusTip(tr("Copy contents to a new dataframe"));
    copyDFAct->setVisible(true);
    copyDFAct->setEnabled(false);
    ui->editToolBar[this]->addAction(copyDFAct);
    connect(copyDFAct, SIGNAL(triggered()), this, SLOT(copyDataframe()));

    plotButton = new QToolButton(this);
    plotButton->setIcon(QIcon(":/images/barchart2.png"));
    plotButton->setVisible(true);
    plotButton->setEnabled(false);
    plotButton->setPopupMode(QToolButton::InstantPopup);
    plotButton->setToolTip("Plot");
    plotMenu = new QMenu(plotButton);
    connect(plotMenu, SIGNAL(aboutToShow()), this, SLOT(buildPlotMenu()));
    plotButton->setMenu(plotMenu);
    plotAct = ui->editToolBar[this]->addWidget(plotButton);

    dataframeViewButton = new QToolButton(this);
    dataframeViewButton->setIcon(QIcon(":/images/R2.png"));
    dataframeViewButton->setVisible(true);
    dataframeViewButton->setEnabled(false);
    dataframeViewButton->setPopupMode(QToolButton::InstantPopup);
    dataframeViewButton->setToolTip("Transform data via R script");
    dataframeViewMenu = new QMenu(dataframeViewButton);
    connect(dataframeViewMenu, SIGNAL(aboutToShow()), this, SLOT(buildDataframeViewMenu()));
    dataframeViewButton->setMenu(dataframeViewMenu);
    dataframeViewAct = ui->editToolBar[this]->addWidget(dataframeViewButton);



}

bool DataframeViewer::partiallyLoaded(QString name)
{
    if (name.isEmpty())
        name = ui->currentItemName();
    if (name.isEmpty() || !SessionManager::instance()->exists(name) || !modelMap[name])
        return false;
    QDomDocument *description = SessionManager::instance()->descriptionCache->getDomDoc(name);
    if (!description)
        return false;
    return XMLelement(*description)["rows"]().toInt() > modelMap[name]->rowCount() ||
            XMLelement(*description)["columns"]().toInt() > modelMap[name]->columnCount();
}

bool DataframeViewer::dataframeExceedsCellLimit(QString name, int maxCells)
{
    if (maxCells < 1)
        return true;
    QDomDocument *description = SessionManager::instance()->descriptionCache->getDomDoc(name);
    if (!description)
        return true;
    int rows = XMLelement(*description)["rows"]().toInt();
    int cols = XMLelement(*description)["columns"]().toInt();
    return rows * cols > maxCells;
}

void DataframeViewer::limitedGet(QString name, int maxCells)
{
    if (maxCells < 1)
        return;
    QDomDocument *description = SessionManager::instance()->description(name);
    if (!description)
        return;
    int rows = XMLelement(*description)["rows"]().toInt();
    int cols = XMLelement(*description)["columns"]().toInt();

    if ((rows > modelMap[name]->rowCount() || cols > modelMap[name]->columnCount())  &&
            (modelMap[name]->rowCount() * modelMap[name]->columnCount() < maxCells - qMin(rows,  cols)))
    {
        QString restriction;
        if (rows * cols > maxCells)
        {
            if (rows <= cols)
            {
                int get_cols = (int)maxCells / rows;
                if (get_cols > 0)
                    restriction = QString("1-%1 1-%2").arg(rows).arg(get_cols);
                else
                    restriction = QString("1-%1 1-1").arg(maxCells);
            }
            else
            {
                int get_rows = (int)maxCells / cols;
                if (get_rows > 0)
                    restriction = QString("1-%1 1-%2").arg(get_rows).arg(cols);
                else
                    restriction = QString("1-1 1-%1").arg(maxCells);
            }
        }
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = QStringList({QString("xml %1 get %2").arg(name).arg(restriction)});;
        job->setAnswerReceiver(SessionManager::instance()->dataframeCache, SLOT(setDomDocAndValidCache(QDomDocument*, QString)), AnswerFormatterType::XML);
        SessionManager::instance()->submitJobs(job);
    }
}

void DataframeViewer::doCopy()
{
    // this is an illegal approach -- get R to copy the df to the clipboard
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("R << write.table(eN[\"%1\"], \"clipboard-100000\", sep=\"\\t\", row.names=FALSE)")
                                .arg(ui->currentItemName())});
    // clipboard-100000 sets the clipboard size to 100000K. Note that a larger number does not work. The default (on Windows) is 33K
    SessionManager::instance()->submitJobs(job);
    // the legal one is:

    //    DataFrameModel *model = modelMap.value(ui->currentItemName());
    //    if (model)
    //        qApp->clipboard()->setText(model->writeTable());
}


void DataframeViewer::dispatch()
{
    DataViewer::dispatch();
}

