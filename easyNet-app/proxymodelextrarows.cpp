#include "proxymodelextrarows.h"

ProxyModelExtraRows::ProxyModelExtraRows(QObject *parent)
    : QIdentityProxyModel(parent)
{
}

void ProxyModelExtraRows::setHeadingItems(QStringList items)
{
    headingItems = items;
    headingCount = headingItems.count();
}

void ProxyModelExtraRows::setTrailingtems(QStringList items)
{
    trailingItems = items;
    trailingCount = trailingItems.count();
}

int ProxyModelExtraRows::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return sourceModel()->rowCount() + headingCount + trailingCount;
}

QModelIndex ProxyModelExtraRows::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) return QModelIndex();
    else if (sourceIndex.parent().isValid()) return QModelIndex();
    return createIndex(sourceIndex.row() + headingCount, sourceIndex.column());
}

QModelIndex ProxyModelExtraRows::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid())
        return QModelIndex();
    else if (proxyIndex.row() < headingCount ||
             proxyIndex.row() >= headingCount + sourceModel()->rowCount())
        return QModelIndex();
    return sourceModel()->index(proxyIndex.row() - headingCount, proxyIndex.column());
}

QVariant ProxyModelExtraRows::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (index.row() < headingCount)
    {
        if (role == Qt::DisplayRole)
            return headingItems.at(index.row());
        else
            return QVariant();
    }
    else if (index.row() >= headingCount + sourceModel()->rowCount())
    {
        if (role == Qt::DisplayRole)
            return trailingItems.at(index.row() - headingCount - sourceModel()->rowCount());
        else
            return QVariant();
    }
    return sourceModel()->data(mapToSource(index), role);
//    return QIdentityProxyModel::data(createIndex(index.row(),index.column()), role);
//        return QIdentityProxyModel::data(index, role);
}

Qt::ItemFlags ProxyModelExtraRows::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    if (index.row() < headingCount ||
            index.row() >= headingCount + sourceModel()->rowCount())
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    return sourceModel()->flags(mapToSource(index));
//      return QIdentityProxyModel::flags(createIndex(index.row(),index.column()));
}

QModelIndex ProxyModelExtraRows::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row > rowCount()) return QModelIndex();
    return createIndex(row, column);
}

//QModelIndex ProxyModelExtraRows::parent(const QModelIndex &child) const
//{
//    Q_UNUSED(child)
//      return QModelIndex();
//}












