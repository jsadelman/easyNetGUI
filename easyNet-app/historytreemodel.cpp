#include "historytreemodel.h"
#include "treeitem.h"
#include "enumclasses.h"


HistoryTreeModel::HistoryTreeModel(QObject *parent)
    : TreeModel(QStringList({""}), parent)
{
}

QVariant HistoryTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() != 0)
        return QVariant();

    CheckRecord record = getItem(index)->data(0).value<CheckRecord>();

    if (!index.parent().isValid())
    {
        if (role == Qt::DisplayRole)
            return record.text;
        else
            return QVariant();
    }
    else
    {
        switch (role)
        {
        case Qt::DisplayRole:
            return record.text;
        case Qt::CheckStateRole:
            return record.checked ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked);
        default:
            return QVariant();
        }
    }
}

Qt::ItemFlags HistoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    else if (!index.parent().isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    else
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

int HistoryTreeModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

bool HistoryTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    TreeItem *item = getItem(index);
    QVariant v = item->data(0);
    CheckRecord record;
    if (v.canConvert<CheckRecord>())
        record = v.value<CheckRecord>();
    bool success = false;
    switch(role)
    {
    case Qt::EditRole:
        record.text = value.toString();
        v.setValue(record);
        success = item->setData(0, v);
        break;
    case Qt::CheckStateRole:
        if (index.parent().isValid())
        {
            record.checked = value.toInt() == Qt::Checked;
            v.setValue(record);
            success = item->setData(0, v);
            if (success)
                emit checkDataChanged();
        }
    default:
        ;
    }
    if (success)
        emit dataChanged(index, index);
    return success;
}

bool HistoryTreeModel::containsTrial(QString trial)
{
    return trialIndex(trial).isValid();
}

bool HistoryTreeModel::containsView(QString view, QString trial)
{
    return viewIndex(view, trial).isValid();
}

bool HistoryTreeModel::appendTrial(QString trial)
{
    if (!insertRows(rowCount(), 1))
    {
        eNerror << "cannot insert a new row for trial" << trial;
        return false;
    }
    if (!setData(index(rowCount()-1,0), trial, Qt::EditRole))
    {
        eNerror << "cannot set trial" << trial;
        return false;
    }
    return true;
}

bool HistoryTreeModel::removeTrial(QString trial)
{
    QModelIndex index = trialIndex(trial);
    if (!index.isValid())
    {
        eNwarning << QString("attempt to remove trial %1, which is not contained in this model").arg(trial);
        return false;
    }
    return removeRows(index.row(), 1);
}

bool HistoryTreeModel::appendView(QString view, QString trial, bool inView)
{
    if (!containsTrial(trial))
        appendTrial(trial);

    QModelIndex trialIdx = trialIndex(trial);

    if (!insertRows(rowCount(trialIdx), 1, trialIdx))
    {
        eNerror << "cannot insert a new row for view" << view;
        return false;
    }

    trialIdx = trialIndex(trial);
    if (!setData(index(rowCount(trialIdx)-1, 0, trialIdx), view, Qt::EditRole))
    {
        eNerror << "cannot set view" << view;
        return false;
    }
    if (!setData(index(rowCount(trialIdx)-1, 0, trialIdx), inView ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked), Qt::CheckStateRole))
    {
        eNerror << "cannot set check value for view" << view;
        return false;
    }
    return true;
}

bool HistoryTreeModel::removeView(QString view, QString trial)
{
    QModelIndex index = viewIndex(view, trial);
    if (!index.isValid())
    {
        eNwarning << QString("attempt to remove view %1 from trial %2, which is not contained in this model").arg(view).arg(trial);
        return false;
    }
    return removeRows(index.row(), 1, trialIndex(trial));
}

QModelIndex HistoryTreeModel::trialIndex(QString trial)
{
    QModelIndexList matchList = match(index(0,0), Qt::DisplayRole, trial, 1, Qt::MatchExactly);
    return matchList.isEmpty() ? QModelIndex() : matchList.first();
}

QModelIndex HistoryTreeModel::viewIndex(QString view, QString trial)
{
    QModelIndex trialIdx = trialIndex(trial);
    if (!trialIdx.isValid())
        return QModelIndex();

    QModelIndexList matchList = match(index(0, 0, trialIdx), Qt::DisplayRole, view, 1, Qt::MatchFixedString | Qt::MatchCaseSensitive);
    return matchList.isEmpty() ? QModelIndex() : matchList.first();
}

