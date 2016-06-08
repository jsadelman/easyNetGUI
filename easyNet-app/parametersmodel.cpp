#include "parametersmodel.h"

ParametersModel::ParametersModel(QDomDocument *domDoc, QObject *parent)
    : DataFrameModel(domDoc, parent)
{
    Q_ASSERT(columnCount() >=2);
}

bool ParametersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() == 0) // can't edit parameter names
            return false;
    // check legality
    bool ok;
    float fvalue = value.toFloat(&ok);
    if (!ok) // not a float
        return false;
    if (role == Qt::EditRole)
    {
        tBody().childNodes().at(index.row() + 1).childNodes().at(index.column() +1).firstChild().setNodeValue(value.toString());
        QString newParamValue = tBody().childNodes().at(index.row() + 1).childNodes().at(index.column()).toElement().text() +
                " " + value.toString();
        emit newParamValueSig(m_name,newParamValue);
    }
    return true;
}

QString ParametersModel::value(QString parameter)
{
    QModelIndexList indList = match(index(0,0), Qt::DisplayRole, parameter, 1, Qt::MatchExactly);
    if (!indList.isEmpty())
        return data(index(indList.first().row(), 1)).toString();

    return QString();
}

Qt::ItemFlags ParametersModel::flags(const QModelIndex &index) const
{
    if (index.column() == 1)
        return DataFrameModel::flags(index) | Qt::ItemIsEditable;

    return DataFrameModel::flags(index);
}
