
#include <QtGui>
#include <QtXml>
//#include <QFont>
//#include <QBrush>

//#include "domitem.h"
#include "lazynutobjtablemodel.h"
#include "lazynutobject.h"



////*********  LazyNutObjTableModel ************//


LazyNutObjTableModel::LazyNutObjTableModel(LazyNutObjectCatalogue *objHash, QObject *parent):
    QAbstractTableModel(parent), _objHash(objHash)
{
}


int LazyNutObjTableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return _objHash->size();
}

int LazyNutObjTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    // name, type
    return 2;
}

QVariant LazyNutObjTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        switch (section)
        {
            case 0:
                return "objects";
            case 1:
                return "type";
            default:
                return QVariant();
        }
    else
        return QVariant();
}


void LazyNutObjTableModel::sendBeginResetModel()
{
    beginResetModel();
}

void LazyNutObjTableModel::sendEndResetModel()
{
    endResetModel();
}



QVariant LazyNutObjTableModel::data(const QModelIndex &index, int role) const
 {
     if (!index.isValid())
         return QVariant();

     if (index.row() >= rowCount())
         return QVariant();

     if (role == Qt::DisplayRole)
     {
         switch(index.column())
         {
            case 0:
                return _objHash->value(_objHash->keys().at(index.row()))->name;
            case 1:
                return _objHash->value(_objHash->keys().at(index.row()))->type;
            default:
                return QVariant();
         }
     }
     else if (role == Qt::ForegroundRole)
     {
           QBrush brush;
           brush.setColor(Qt::blue);
           return brush;
     }
     else
         return QVariant();
}

//*********  LazyNutObjTableProxyModel ************//


LazyNutObjTableProxyModel::LazyNutObjTableProxyModel(QObject *parent):
    QSortFilterProxyModel(parent)
{
}

void LazyNutObjTableProxyModel::setFilterKeyColumns(const QList<int> &filterColumns)
{
    // not used
    _columnPatterns.clear();

    foreach(int column, filterColumns)
        _columnPatterns.insert(column, QString());
}

void LazyNutObjTableProxyModel::addFilterFixedString(int column, const QString &pattern)
{
    // not used
    if(!_columnPatterns.contains(column))
        return;

    _columnPatterns[column] = pattern;
}

void LazyNutObjTableProxyModel::setFilterFromGenealogy(const QList<QVariant> genealogy)
{
    // Coincidentally, positions 1 and 2 in a genealogy (type and subtype)
    // correspond to the same column indices in a LazyNutObjTableModel,
    // while position 0 in a genealogy is always "object", which means no filter.
    // CHANGED: now object not there, so start from 0
    _columnPatterns.clear();
    for (int i = 0; i < genealogy.size(); ++ i)
    {
        _columnPatterns.insert((i+1),genealogy.at(i).toString());
    }
    invalidateFilter();
}

void LazyNutObjTableProxyModel::mapIndexToSourceModel(const QModelIndex &index)
{
    emit getObj(mapToSource(index));
}



bool LazyNutObjTableProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    // if no filter is defined, show all.
    // Else, implement an AND of all the filtering criteria,
    // e.g. type == "layer" AND subtype == "iac_layer".
    // It is clearly redundant, since columnPatterns come from a genealogy,
    // but it is open to other uses.

    if(_columnPatterns.isEmpty())
        return true;

    bool ret = false;

    for(QMap<int, QString>::const_iterator iter = _columnPatterns.constBegin();
                iter != _columnPatterns.constEnd();
                ++iter)
    {
        QModelIndex index = sourceModel()->index(sourceRow, iter.key());
        ret = (index.data().toString() == iter.value());
        if(!ret)
            return ret;
    }

    return ret; // true
}





LazyNutObjectListModel::LazyNutObjectListModel(LazyNutObjectCatalogue *objHash, QObject *parent)
    : QAbstractListModel(parent), _objHash(objHash)
{
}

int LazyNutObjectListModel::rowCount(const QModelIndex &parent) const
{
    return _objHash->size();
}

QVariant LazyNutObjectListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
           case 0:
               return _objHash->value(_objHash->keys().at(index.row()))->name;
           default:
               return QVariant();
        }
    }
    else if (role == Qt::ForegroundRole)
    {
          QBrush brush;
          brush.setColor(Qt::blue);
          return brush;
    }
    else
        return QVariant();
}

QVariant LazyNutObjectListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        switch (section)
        {
            case 0:
                return "objects";
            default:
                return QVariant();
        }
    else
        return QVariant();
}


void LazyNutObjectListModel::sendBeginResetModel()
{
    beginResetModel();
}

void LazyNutObjectListModel::sendEndResetModel()
{
    endResetModel();
}

void LazyNutObjectListModel::getObjFromListIndex(const QModelIndex &index)
{
    emit objectRequested(data(index,Qt::DisplayRole).toString());
}

