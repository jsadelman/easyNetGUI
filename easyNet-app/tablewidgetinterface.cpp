#include "tablewidgetinterface.h"
#include "objectcataloguefilter.h"
#include "dataframemodel.h"
#include "trialdataframemodel.h"
//#include "lazynutjob.h"


TableWidgetInterface::TableWidgetInterface(QWidget *parent)
    : m_filter(nullptr), QWidget(parent)
{

}

TableWidgetInterface::~TableWidgetInterface()
{
    delete m_filter;
}

void TableWidgetInterface::prepareToUpdateTable(QDomDocument *domDoc, QString cmd)
{
    if (m_filter)
    {
        delete domDoc;
        return;
    }

    lastName = nameFromCmd(cmd);
    lastModel = new DataFrameModel(domDoc, this);
    lastModel->setName(lastName);
}

void TableWidgetInterface::setPrettyHeaderFromJob()
{
#if 0
    if (m_filter)
        return;

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
#endif
}

DataFrameModel *TableWidgetInterface::getDataFrameModel(QAbstractItemModel *model)
{
    DataFrameModel *dFmodel = qobject_cast<DataFrameModel *>(model);
    TrialDataFrameModel *tdFmodel = qobject_cast<TrialDataFrameModel *>(model);
    if (tdFmodel)
        dFmodel = qobject_cast<DataFrameModel *>(tdFmodel->sourceModel());
    return dFmodel;
}

void TableWidgetInterface::setFilter(ObjectCacheFilter *filter)
{
    m_filter = filter;
    setFilter_impl();
}

void TableWidgetInterface::updateTable(QDomDocument *domDoc, QString cmd)
{
    if (m_filter)
    {
        delete domDoc;
        return;
    }
    prepareToUpdateTable(domDoc, cmd);
    updateTable_impl(lastModel);
}

void TableWidgetInterface::deleteTable(QString name)
{
    if (m_filter)
        return;

    deleteTable_impl(name);
}

QString TableWidgetInterface::nameFromCmd(QString cmd)
{
    cmd.remove(QRegExp("^\\s*xml| get\\s*$| get .*$"));
    return cmd.simplified();
}

