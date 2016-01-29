#include "historymodel.h"

CheckListModel::CheckListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int CheckListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_history.count();
}

QVariant CheckListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
        return m_history.value(index.row()).name;
    case Qt::CheckStateRole:
        return m_history.value(index.row()).checked ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked);
    default:
        return QVariant();
    }
    return QVariant();
}

//QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if (role != Qt::DisplayRole)
//        return QVariant();

//    if (orientation == Qt::Horizontal) {
//        switch (section) {
//        case 0:
//            return "name";
//        case 1:
//            return "in viewer";
//        default:
//            return QVariant();
//        }
//    }
//    return QVariant();
//}

bool CheckListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && index.row() >= 0 && index.row() < m_history.count())
    {
        switch(role)
        {
        case Qt::EditRole:
            m_history[index.row()].name = value.toString();
            break;
        case Qt::CheckStateRole:
            m_history[index.row()].checked = value.toInt() == Qt::Checked;
            emit checkDataChanged();
            break;
        default:
            return false;
        }
        emit(dataChanged(index, index));
        return true;
    }
    return false;
}

Qt::ItemFlags CheckListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

bool CheckListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (row < 0 || row >= m_history.count() || row+count > m_history.count())
        return false;

    beginRemoveRows(QModelIndex(), row, row+count-1);
    QList<HistoryRecord>::iterator from = m_history.begin() + row;
    QList<HistoryRecord>::iterator to = m_history.begin() + row + count;
    m_history.erase(from, to);
    endRemoveRows();
    return true;
}

void CheckListModel::clear()
{
    if (rowCount() < 1)
        return;
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    m_history.clear();
    endRemoveRows();
}

bool CheckListModel::contains(const QString &name)
{
    return rowFromName(name) > -1;
}

bool CheckListModel::isInViewer(const QString &name)
{
    int row = rowFromName(name);
    if (row > -1)
        return m_history.value(row).checked;
    else
        return false;
}

QStringList CheckListModel::selectedItems()
{
    QStringList items;
    foreach(QModelIndex index, match(index(0), Qt::CheckStateRole, Qt::Checked, -1))
           items.append(index.data(Qt::DisplayRole).toString());
    return items;
}

bool CheckListModel::create(const QString &name)
{
    if (rowFromName(name) >= 0) // name exists already
        return false;

    beginInsertRows(QModelIndex(), 0, 0);
    HistoryRecord record;
    record.name = name;
    record.checked = false;
    m_history.insert(0, record);
    endInsertRows();
    emit dataChanged(index(0,0), index(0,0));
    return true;
}

bool CheckListModel::destroy(const QString &name)
{
    return removeRow(rowFromName(name));
}

int CheckListModel::rowFromName(const QString &name)
{
    QModelIndexList list = match(index(0,0), Qt::DisplayRole, name, 1, Qt::MatchExactly);
    if (!list.isEmpty())
        return list.at(0).row();
    else
        return -1;
}


