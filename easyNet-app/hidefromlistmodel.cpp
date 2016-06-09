#include "hidefromlistmodel.h"

HideFromListModel::HideFromListModel(QObject *parent)
    : relevantColumn(0), QSortFilterProxyModel(parent)
{
    hiddenItems.clear();
}

bool HideFromListModel::hideItem(QVariant item)
{
    QModelIndexList indList = sourceModel()->match(sourceModel()->index(0, relevantColumn), Qt::DisplayRole, item, 1, Qt::MatchExactly);
    if (indList.isEmpty())
        return false;

    hiddenItems.append(item);
    invalidateFilter();
    return true;
}

bool HideFromListModel::showItem(QVariant item)
{
    int removed = hiddenItems.removeAll(item);
    if (removed == 0)
        return false;

    invalidateFilter();
    return true;
}

bool HideFromListModel::showAll()
{
    if (hiddenItems.isEmpty())
        return false;

    hiddenItems.clear();
    invalidateFilter();
    return true;
}

bool HideFromListModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    return source_column == relevantColumn;
}

bool HideFromListModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    return !hiddenItems.contains(sourceModel()->data(sourceModel()->index(source_row, relevantColumn, source_parent), Qt::DisplayRole));
}
