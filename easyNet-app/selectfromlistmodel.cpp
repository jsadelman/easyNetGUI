#include "selectfromlistmodel.h"
#include <QDebug>

SelectFromListModel::SelectFromListModel(QObject *parent)
    : relevantColumn(0), QAbstractTableModel(parent)
{
}

int SelectFromListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return sourceModel->rowCount();
}

int SelectFromListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

QVariant SelectFromListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    {
        QString key = sourceModel->data(sourceModel->index(index.row(),relevantColumn)).toString();

        switch (index.column())
        {
        case 0:
            return key;
        case 1:
        {
//            qDebug () << key << isSelected[key];
            return isSelected[key];
        }
        default:
            return QVariant();
        }
    }
    default:
        return QVariant();
    }
}

Qt::ItemFlags SelectFromListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    switch (index.column())
    {
    case 0:
        return flags;
    case 1:
        return flags | Qt::ItemIsEditable;
    default:
        return Qt::NoItemFlags;
    }
}

bool SelectFromListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    switch (role)
    {
    case Qt::EditRole:
    {
        switch (index.column())
        {
        case 0:
            return false;
        case 1:
        {
            QString key = data(this->index(index.row(),0)).toString();
            isSelected[key] = value.toBool();
            emit dataChanged(index, index);
            return true;
        }
        default:
            return false;
        }
    }
    default:
        return false;
    }
}

QModelIndex SelectFromListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row <0 || row >= rowCount())
        return QModelIndex();
    switch (column)
    {
    case 0:
    case 1:
        return createIndex(row, column);
    default:
        return QModelIndex();
    }
}

void SelectFromListModel::setSourceModel(QAbstractItemModel *model)
{
    sourceModel = model;
//    connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
//            this, SLOT(updateNewRows(QModelIndex,int,int)));
//    connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
//            this, SLOT(updateRemovedRows(QModelIndex,int,int)));
//    connect(sourceModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int> &)),
//            this, SLOT(updateRows(const QModelIndex&, const QModelIndex&, const QVector<int> &)));
}

QModelIndexList SelectFromListModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    // limit search to the first column, corresponding to the relevant column of the source model
    if (start.column() != 0)
        return QModelIndexList();
    QModelIndexList sourceIndexes = sourceModel->match(sourceModel->index(start.row(), relevantColumn), role, value, hits, flags);
    QModelIndexList indexes;
    foreach (QModelIndex idx, sourceIndexes)
        indexes.append(index(idx.row(),0));
    return indexes;
}

void SelectFromListModel::updateNewRows(QModelIndex parent, int first, int last)
{
    beginInsertRows(parent, first, last);
    for (int row = first; row <= last; ++row)
    {
        QString key = sourceModel->data(sourceModel->index(row, relevantColumn)).toString();
        isSelected[key] = false;
//        setData(index(row, 0), key);
//        setData(index(row, 1), false);
    }
    endInsertRows();
}

void SelectFromListModel::updateRemovedRows(QModelIndex parent, int first, int last)
{
    beginRemoveRows(parent, first, last);
    for (int row = first; row <= last; ++row)
        isSelected.remove(data(index(row, 0)).toString());
    endRemoveRows();
}

void SelectFromListModel::updateRows(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
//    qDebug() << "dataChanged";
    emit dataChanged(index(topLeft.row(),0), index(bottomRight.row(),0),roles);
}
