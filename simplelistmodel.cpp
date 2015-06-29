#include "simplelistmodel.h"
#include <QSet>
#include <QDebug>

StringListModel::StringListModel(QStringList list, QObject *parent)
    : list(list), QAbstractListModel(parent)
{
}

int StringListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return list.size();
}

QVariant StringListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() != 0)
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
        return list.at(index.row());
    default:
        return QVariant();
    }
}

bool StringListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (index.column() != 0)
        return false;

    if (index.row() < 0 || index.row() >= list.size())
        return false;

    switch (role)
    {
    case Qt::EditRole:
        list[index.row()] = value.toString();
        emit dataChanged(index, index);
        return true;
    default:
        return false;
    }
}

Qt::ItemFlags StringListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    if (index.column() != 0)
        return 0;

    if (index.row() < 0 || index.row() >= list.size())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool StringListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row > rowCount() || count <= 0)
        return false;
    beginInsertRows(parent, row, row + count -1);
    for (int i = row; i < row + count; ++i)
        list.insert(i, QString());
    endInsertRows();
    return true;
}

bool StringListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row >= rowCount() || count <= 0 || row + count -1 >= rowCount())
        return false;
    beginRemoveRows(parent, row, row + count -1);
    QStringList::iterator from = list.begin() + row;
    QStringList::iterator to = list.begin() + row + count;
    list.erase(from, to);
    endRemoveRows();
    return true;
}

bool StringListModel::appendStrings(QStringList txtList)
{
    bool success = true;
    success &= insertRows(list.count(), txtList.count());
    for (int i = list.count() - txtList.count(); i < list.count(); ++i)
        success &= setData(index(i), txtList.at(i - (list.count() - txtList.count())));
    return success;
}

bool StringListModel::appendString(QString txt)
{
    return appendStrings(QStringList({txt}));
}

bool StringListModel::removeString(QString txt)
{
    return removeRows(list.indexOf(txt), 1);
}

bool StringListModel::removeStrings(QStringList txtList)
{
    bool success = true;
    foreach (QString txt, txtList)
        success &= removeString(txt);
    return success;
}

bool StringListModel::updateList(QStringList newList)
{
    qDebug() << "StringListModel::updateList";
    bool success = true;
    QSet<QString> setDiff;
    // append new items
    setDiff = newList.toSet() - list.toSet();
    success &= appendStrings(setDiff.toList());
    // remove old items
    setDiff = list.toSet() -  newList.toSet();
    success &= removeStrings(setDiff.toList());
    return success;
}
