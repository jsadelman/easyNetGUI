#include "parametersproxymodel.h"

#include "dataframemodel.h"
#include "enumclasses.h"


ParametersProxyModel::ParametersProxyModel(QObject *parent)
    : QIdentityProxyModel(parent), m_referenceModel(nullptr)
{
}

void ParametersProxyModel::setSourceModel(QAbstractItemModel *model)
{
    DataFrameModel *dfModel = qobject_cast<DataFrameModel *>(model);
    if (dfModel)
        QIdentityProxyModel::setSourceModel(dfModel);
    else
        eNerror << "source model should be a DataFrameModel";
}

void ParametersProxyModel::setReferenceModel(DataFrameModel *model)
{
    if (model)
        m_referenceModel = model;
    else
        eNerror << "reference model should be a DataFrameModel";
}

QVariant ParametersProxyModel::data(const QModelIndex &ind, int role) const
{
    if (role == Qt::ForegroundRole && m_referenceModel)
    {
        QVariant param = sourceModel()->data(index(ind.row(),0));
        QVariant val = sourceModel()->data(index(ind.row(),1));
        QModelIndexList idxList = m_referenceModel->match(m_referenceModel->index(0, 0), Qt::DisplayRole, param, 1, Qt::MatchExactly);
        if (idxList.isEmpty() || // new parameter
                (ind.column() == 1 &&
                 m_referenceModel->data(m_referenceModel->index(idxList.first().row(),1)) != val)) // different values
            return QVariant(QColor(Qt::red));
        else
            return sourceModel()->data(ind, role);
    }
    else
        return sourceModel()->data(ind, role);
}

// Qt::ForegroundRole
