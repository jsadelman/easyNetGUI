#include "tablewidgetinterface.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "dataframemodel.h"
#include "trialdataframemodel.h"
#include "lazynutjob.h"
#include "sessionmanager.h"

#include <QDebug>


TableWidgetInterface::TableWidgetInterface(QWidget *parent)
    : headerReplaceRules(), QWidget(parent)
{
    dataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    dataframeUpdater = new ObjectUpdater(this);
    dataframeUpdater->setCommand("get");
    dataframeUpdater->setProxyModel(dataframeFilter);
//    connect(this, SIGNAL(currentTableChanged(QString)),
//            dataframeFilter, SLOT(setName(QString)));
    connect(dataframeUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)),
            this, SLOT(updateTable(QDomDocument*,QString)));
    dataframeDescriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    dataframeDescriptionUpdater = new ObjectUpdater(this);
    dataframeDescriptionUpdater->setProxyModel(dataframeDescriptionFilter);

}

TableWidgetInterface::~TableWidgetInterface()
{
    delete dataframeFilter;
}

void TableWidgetInterface::addHeaderReplaceRules(Qt::Orientation orientation, QString from, QString to)
{
    headerReplaceRules[orientation].append(QPair<QString, QString>(from, to));
}

bool TableWidgetInterface::contains(QString table)
{
    return modelMap.contains(table);
}

void TableWidgetInterface::setPrettyHeaders(QString tableName, TrialDataFrameModel* prettyHeadersModel)
{
    prettyHeadersModelMap.insert(tableName, prettyHeadersModel);
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
    // to be deleted
    lastName = nameFromCmd(cmd);
    lastModel = new DataFrameModel(domDoc, this);
    lastModel->setName(lastName);
}

//void TableWidgetInterface::setPrettyHeaderFromJob()
//{
//    // to be deleted
//   LazyNutJob *job = qobject_cast<LazyNutJob *>(sender());
//    TrialDataFrameModel *trialDataFrameModel = new TrialDataFrameModel(this);
//    if (job)
//    {
//        QMapIterator<QString, QVariant> headerReplaceHorizontalIt(job->data.toMap());
//        while (headerReplaceHorizontalIt.hasNext())
//        {
//            headerReplaceHorizontalIt.next();
//            trialDataFrameModel->addHeaderReplaceRules(
//                        Qt::Horizontal,
//                        headerReplaceHorizontalIt.key(),
//                        headerReplaceHorizontalIt.value().toString());
//        }
//    }

//    trialDataFrameModel->setSourceModel(lastModel);
//    modelMap[lastName] = trialDataFrameModel;
//    updateTable_impl(trialDataFrameModel);
//}

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
    DataFrameModel *dfModel = new DataFrameModel(domDoc, this);
    QString name = nameFromCmd(cmd);
    dfModel->setName(name);
    TrialDataFrameModel *prettyHeadersModel = nullptr;
    if (prettyHeadersModelMap.contains(name))
    {
        prettyHeadersModel = prettyHeadersModelMap.value(name);
        prettyHeadersModel->setSourceModel(dfModel);
    }
    if (prettyHeadersModel)
        updateTable_impl(prettyHeadersModel);
    else
        updateTable_impl(dfModel);
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

