#include "selectfromlistmodel.h"

SelectFromListModel::SelectFromListModel(QObject *parent)
    : relevantColumn(0), QIdentityProxyModel(parent)
{
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
        QString key = sourceModel()->data(sourceModel()->index(index.row(),relevantColumn)).toString();
        switch (index.column())
        {
        case 0:
            return key;
        case 1:
            return isSelected[key];
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
    QIdentityProxyModel::setSourceModel(model);
    connect(sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(updateNewRows(QModelIndex,int,int)));
    connect(sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(updateRemovedRows(QModelIndex,int,int)));
}

void SelectFromListModel::updateNewRows(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)
    for (int row = first; row <= last; ++row)
    {
        isSelected[data(index(row, 0)).toString()] = false;
        setData(index(row, 1), false);
    }
}

void SelectFromListModel::updateRemovedRows(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)
    for (int row = first; row <= last; ++row)
        isSelected.remove(data(index(row, 0)).toString());
}
