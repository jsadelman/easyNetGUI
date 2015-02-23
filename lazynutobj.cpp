#include "lazynutobj.h"
#include "lazynutobject.h"

#include <QFont>
#include <QBrush>


/*
LazyNutProperty::LazyNutProperty(const QString &label, const QVariant &value): property(label, value) {}


LazyNutObj::LazyNutObj(const QString &name, const QString &type, const QString &subtype)
{
    properties << new LazyNutProperty("name", name);
    properties << new LazyNutProperty("type", type);
    properties << new LazyNutProperty("subtype", subtype);
}

LazyNutObj::~LazyNutObj()
{
    qDeleteAll(properties);
}

QString LazyNutObj::name() const
{
    if (properties.at(0)->property.first != "name")
        return QString();
    return properties.at(0)->property.second.toString();
}

QString LazyNutObj::type() const
{
    if (properties.at(1)->property.first != "type")
        return QString();
    return properties.at(1)->property.second.toString();
}

QString LazyNutObj::subtype() const
{
    if (properties.at(2)->property.first != "subtype")
        return QString();
    return properties.at(2)->property.second.toString();
}

QList<int> LazyNutObj::rowsWithObjects()
{
    QList<int> rows;
    for (int i = 0; i < properties.size(); ++i)
        if (!properties.at(i)->objects.isEmpty())
            rows << i;
    return rows;
}

QList<int> LazyNutObj::rowsWithMultipleObjects()
{
    QList<int> rows;
    for (int i = 0; i < properties.size(); ++i)
        if (properties.at(i)->objects.size()>1)
            rows << i;
    return rows;
}

QList<int> LazyNutObj::rowsWithStringList()
{
    QList<int> rows;
    for (int i = 0; i < properties.size(); ++i)
        if (properties.at(i)->property.second.type() == QVariant::StringList)
            rows << i;
    return rows;
}

QList<int> LazyNutObj::rowsWithMultipleItems()
{
    QList<int> rows;
    for (int i = 0; i < properties.size(); ++i)
        if (properties.at(i)->property.second.type() == QVariant::StringList || properties.at(i)->objects.size()>1)
            rows << i;
    return rows;
}
*/

//*********  LazyNutObj ************//


//void LazyNutObj::appendProperty(const QString &key, const QVariant &value)
//{
//    _properties.append(qMakePair(key,value));
//    _keyAt.insert(key, _properties.size() -1);
//    if (key == "name")
//        _name = value.toString();
//    else if (key == "type")
//        _type = value.toString();
//    else if (key == "subtype")
//        _subtype = value.toString();
//}

//QVariant LazyNutObj::getValue(const QString& key)
//{
//    if (_keyAt.contains(key))
//        return _properties.at(_keyAt.value(key)).second;
//    return QVariant();
//}

//QVariant LazyNutObj::getValue(int i)
//{
//    return _properties.at(i).second;
//}

//QString LazyNutObj::getKey(int i)
//{
//    return _properties.at(i).first;
//}

//QPair<QString, QVariant> LazyNutObj::getProperty(int i)
//{
//    return _properties.at(i);
//}

//int LazyNutObj::rowCount()
//{
//    return _properties.size();
//}

//QString& LazyNutObj::name() {return _name;}
//QString& LazyNutObj::type() {return _type;}
//QString& LazyNutObj::subtype() {return _subtype;}

//QList<int> LazyNutObj::rowsWithStringList()
//{
//    // returns a list of positions corresponding to properties the values of which
//    // are of type QStringList.
//    QList<int> rows;
//    for (int i = 0; i < _properties.size(); ++i)
//        if (_properties.at(i).second.type() == QVariant::StringList)
//            rows << i;
//    return rows;
//}

//*********  LazyNutObjModel ************//



//int LazyNutObjModel::rowCount(const QModelIndex &/*parent*/) const
//{
//    return _obj->rowCount();
//}

//QVariant LazyNutObjModel::data(const QModelIndex &index, int role) const
// {
//     if (!index.isValid())
//         return QVariant();

//     if (index.row() >= _obj->rowCount())
//         return QVariant();

//     switch (role)
//     {
//        case Qt::DisplayRole:
//            return _obj->getValue(index.row());
//        case Qt::ForegroundRole:
//        {
//            QBrush brush;
//            if (_objHash->contains(_obj->getValue(index.row()).toString()))
//                brush.setColor(Qt::blue);
//            return brush;
//        }
//        default:
//            return QVariant();
//     }
// }

//QVariant LazyNutObjModel::headerData(int section, Qt::Orientation orientation, int role) const
// {
//    if (orientation == Qt::Horizontal)
//        switch (section)
//        {
//            case 0:
//                return QString("Property");
//            case 1:
//                 return QString("Value");
//            default:
//                return QVariant();
//        }

//    else if (orientation == Qt::Vertical)
//    {
//        switch (role)
//        {
//            case Qt::DisplayRole:
//                return _obj->getKey(section);
//            case Qt::FontRole:
//            {
//                QFont boldFont;
//                boldFont.setBold(true);
//                return boldFont;
//            }
//            default:
//                return QVariant();
//        }
//    }
//    else
//        return QVariant();
//}

//bool LazyNutObjModel::isNameInCatalogue(const QString &name)
//{
//    return _objHash->contains(name);
//}

//void LazyNutObjModel::getObjFromDescriptionIndex(const QModelIndex &index)
//{
//    if(_objHash->contains(_obj->getValue(index.row()).toString()))
//        emit showObj(_objHash->value(_obj->getValue(index.row()).toString()),_objHash);
//}

//void LazyNutObjModel::getObjFromDescriptionIndex(const QModelIndex &index, int objIndex)
//{
//    if(_objHash->contains(_obj->getValue(index.row()).toStringList().at(objIndex)))
//        emit showObj(_objHash->value(_obj->getValue(index.row()).toStringList().at(objIndex)),_objHash);
//}

//void LazyNutObjModel::getObjFromCatalogueIndex(const QModelIndex &index)
//{
//    if (_objHash->contains(index.data().toString()))
//        emit showObj(_objHash->value(index.data().toString()),_objHash);
//}

////*********  LazyNutObjTableModel ************//


//LazyNutObjTableModel::LazyNutObjTableModel(LazyNutObjCatalogue *objHash, QObject *parent):
//    QAbstractTableModel(parent), _objHash(objHash)
//{
//}


//int LazyNutObjTableModel::rowCount(const QModelIndex &/*parent*/) const
//{
//    return _objHash->size();
//}

//int LazyNutObjTableModel::columnCount(const QModelIndex &/*parent*/) const
//{
//    // name, type, subtype
//    return 3;
//}

//QVariant LazyNutObjTableModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if (role != Qt::DisplayRole)
//        return QVariant();

//    if (orientation == Qt::Horizontal)
//        switch (section)
//        {
//            case 0:
//                return "objects";
//            case 1:
//                return "type";
//            case 2:
//                return "subtype";
//            default:
//                return QVariant();
//        }
//    else
//        return QVariant();
//}


//void LazyNutObjTableModel::sendBeginResetModel()
//{
//    beginResetModel();
//}

//void LazyNutObjTableModel::sendEndResetModel()
//{
//    endResetModel();
//}



//QVariant LazyNutObjTableModel::data(const QModelIndex &index, int role) const
// {
//     if (!index.isValid())
//         return QVariant();

//     if (index.row() >= rowCount())
//         return QVariant();

//     if (role == Qt::DisplayRole)
//     {
//         switch(index.column())
//         {
//            case 0:
//                return _objHash->value(_objHash->keys().at(index.row()))->name();
//            case 1:
//                return _objHash->value(_objHash->keys().at(index.row()))->type();
//            case 2:
//                return _objHash->value(_objHash->keys().at(index.row()))->subtype();
//            default:
//                return QVariant();
//         }
//     }
//     else if (role == Qt::ForegroundRole)
//     {
//           QBrush brush;
//           brush.setColor(Qt::blue);
//           return brush;
//     }
//     else
//         return QVariant();
//}

////*********  LazyNutObjTableProxyModel ************//


//LazyNutObjTableProxyModel::LazyNutObjTableProxyModel(QObject *parent):
//    QSortFilterProxyModel(parent)
//{
//}

//void LazyNutObjTableProxyModel::setFilterKeyColumns(const QList<int> &filterColumns)
//{
//    // not used
//    _columnPatterns.clear();

//    foreach(int column, filterColumns)
//        _columnPatterns.insert(column, QString());
//}

//void LazyNutObjTableProxyModel::addFilterFixedString(int column, const QString &pattern)
//{
//    // not used
//    if(!_columnPatterns.contains(column))
//        return;

//    _columnPatterns[column] = pattern;
//}

//void LazyNutObjTableProxyModel::setFilterFromGenealogy(const QList<QVariant> genealogy)
//{
//    // Coincidentally, positions 1 and 2 in a genealogy (type and subtype)
//    // correspond to the same column indices in a LazyNutObjTableModel,
//    // while position 0 in a genealogy is always "object", which means no filter.
//    _columnPatterns.clear();
//    for (int column = 1; column < genealogy.size(); ++ column)
//    {
//        _columnPatterns.insert(column,genealogy.at(column).toString());
//    }
//    invalidateFilter();
//}

//void LazyNutObjTableProxyModel::mapIndexToSourceModel(const QModelIndex &index)
//{
//    emit getObj(mapToSource(index));
//}



//bool LazyNutObjTableProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
//{
//    // if no filter is defined, show all.
//    // Else, implement an AND of all the filtering criteria,
//    // e.g. type == "layer" AND subtype == "iac_layer".
//    // It is clearly redundant, since columnPatterns come from a genealogy,
//    // but it is open to other uses.

//    if(_columnPatterns.isEmpty())
//        return true;

//    bool ret = false;

//    for(QMap<int, QString>::const_iterator iter = _columnPatterns.constBegin();
//                iter != _columnPatterns.constEnd();
//                ++iter)
//    {
//        QModelIndex index = sourceModel()->index(sourceRow, iter.key());
//        ret = (index.data().toString() == iter.value());
//        if(!ret)
//            return ret;
//    }

//    return ret; // true
//}





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
               return _objHash->value(_objHash->keys().at(index.row()))->name();
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
    // this signal is protected
    beginResetModel();
}

void LazyNutObjectListModel::sendEndResetModel()
{
    // this signal is protected
    endResetModel();
}

void LazyNutObjectListModel::getObjFromListIndex(const QModelIndex &index)
{
    emit objectRequested(data(index,Qt::DisplayRole).toString());
}
