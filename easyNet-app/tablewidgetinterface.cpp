#include "tablewidgetinterface.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "dataframemodel.h"
#include "trialdataframemodel.h"
#include "lazynutjob.h"
#include "sessionmanager.h"

#include <QDebug>


TableWidgetInterface::TableWidgetInterface(QWidget *parent)
    : QWidget(parent)
{
    dataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    dataframeUpdater = new ObjectUpdater(this);
    dataframeUpdater->setCommand("get");
    dataframeUpdater->setProxyModel(dataframeFilter);
    connect(this, SIGNAL(currentTableChanged(QString)),
            dataframeFilter, SLOT(setName(QString)));
    connect(dataframeUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)),
            this, SLOT(updateTable(QDomDocument*,QString)));
}

TableWidgetInterface::~TableWidgetInterface()
{
    delete dataframeFilter;
}

void TableWidgetInterface::addTable(QString name)
{
    if (modelMap.contains(name))
    {
        qDebug() << "ERROR: TableWidgetInterface::addTable attempt to create existing tab" << name;
        return;
    }
    modelMap.insert(name, nullptr);
    addTable_impl(name);
}

void TableWidgetInterface::prepareToUpdateTable(QDomDocument *domDoc, QString cmd)
{
    lastName = nameFromCmd(cmd);
    lastModel = new DataFrameModel(domDoc, this);
    lastModel->setName(lastName);
}

void TableWidgetInterface::setPrettyHeaderFromJob()
{
   LazyNutJob *job = qobject_cast<LazyNutJob *>(sender());
    TrialDataFrameModel *trialDataFrameModel = new TrialDataFrameModel(this);
    if (job)
    {
        QMapIterator<QString, QVariant> headerReplaceHorizontalIt(job->data.toMap());
        while (headerReplaceHorizontalIt.hasNext())
        {
            headerReplaceHorizontalIt.next();
            trialDataFrameModel->addHeaderReplace(
                        Qt::Horizontal,
                        headerReplaceHorizontalIt.key(),
                        headerReplaceHorizontalIt.value().toString());
        }
    }

    trialDataFrameModel->setSourceModel(lastModel);
    modelMap[lastName] = trialDataFrameModel;
    updateTable_impl(trialDataFrameModel);
}

DataFrameModel *TableWidgetInterface::getDataFrameModel(QAbstractItemModel *model)
{
    DataFrameModel *dFmodel = qobject_cast<DataFrameModel *>(model);
    TrialDataFrameModel *tdFmodel = qobject_cast<TrialDataFrameModel *>(model);
    if (tdFmodel)
        dFmodel = qobject_cast<DataFrameModel *>(tdFmodel->sourceModel());
    return dFmodel;
}


void TableWidgetInterface::updateTable(QDomDocument *domDoc, QString cmd)
{
    prepareToUpdateTable(domDoc, cmd);
    updateTable_impl(lastModel);
}

void TableWidgetInterface::deleteTable(QString name)
{
     deleteTable_impl(name);
}

QString TableWidgetInterface::nameFromCmd(QString cmd)
{
    cmd.remove(QRegExp("^\\s*xml| get\\s*$| get .*$"));
    return cmd.simplified();
}

