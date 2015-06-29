#include <QtDebug>
#include "dataframemodel.h"


DataFrameModel::DataFrameModel(QDomDocument *domDoc, QObject *parent)
    :domDoc(domDoc), QAbstractTableModel(parent)
{
}

int DataFrameModel::rowCount(const QModelIndex &parent) const
{
    return tBody().childNodes().size() - 1;
}

int DataFrameModel::columnCount(const QModelIndex &parent) const
{
    return tBody().firstChild().childNodes().size() - 1;
}

QVariant DataFrameModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        return tBody().childNodes().at(index.row() + 1).childNodes().at(index.column() +1).toElement().text();
    }
    return QVariant();
}

QVariant DataFrameModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return tBody().firstChild().childNodes().at(section +1).toElement().text();

    else if (orientation == Qt::Vertical && role == Qt::DisplayRole)
        return tBody().childNodes().at(section +1).firstChildElement().text();

    return QVariant();
}

QStringList DataFrameModel::rowNames()
{
    QStringList rows;
    for (int i = 0; i < rowCount(); ++i)
        rows.append(headerData(i, Qt::Vertical, Qt::DisplayRole).toString());
    return rows;
}

QStringList DataFrameModel::colNames()
{
    QStringList cols;
    for (int i = 0; i < columnCount(); ++i)
        cols.append(headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
    return cols;
}

bool DataFrameModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.column() == 0) // can't edit parameter names
            return true;
    // check legality
    bool ok;
    float fvalue = value.toFloat(&ok);
    if (!ok) // not a float
        return true;
    if (role == Qt::EditRole)
    {
        tBody().childNodes().at(index.row() + 1).childNodes().at(index.column() +1).firstChild().setNodeValue(value.toString());
        QString newParamValue = tBody().childNodes().at(index.row() + 1).childNodes().at(index.column()).toElement().text() +
                " " + value.toString();
        emit newParamValueSig(newParamValue);

    }
    return true;
}




Qt::ItemFlags DataFrameModel::flags (const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}
