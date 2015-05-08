
#include <QtGui>
#include <QtXml>
#include <QFont>
#include <QBrush>

#include "domitem.h"
#include "lazynutobject.h"

LazyNutObject::LazyNutObject(QDomDocument *doc, QObject *parent)
    : domDoc(doc), QAbstractItemModel(parent)
{
    rootItem = new DomItem(domDoc, 0);
    initProperties();
}

void LazyNutObject::initProperties()
{
    QDomNode objectNode = domDoc->firstChild().firstChild();
    while (!objectNode.isNull())
    {
        QString label = objectNode.toElement().attribute("label");
        QString value = objectNode.toElement().attribute("value");
        if (label == "this")
            _name = value;
        else if (label == "type")
        {
            _type = value.section('/',0,0);
            _subtype = value.section('/',1,1);
        }
        objectNode = objectNode.nextSibling();
    }
}

LazyNutObject::~LazyNutObject()
{
    delete rootItem;
    delete domDoc;
}

int LazyNutObject::columnCount(const QModelIndex &/*parent*/) const
{
    return 2;
}

QString LazyNutObject::getValue(QString label)
{
    QDomNode objectNode = domDoc->firstChild().firstChild();
    while (!objectNode.isNull())
    {
        if (objectNode.toElement().attribute("label") == label)
            return objectNode.toElement().attribute("value");
        objectNode = objectNode.nextSibling();
    }
    return QString();
}



QVariant LazyNutObject::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    DomItem *item = static_cast<DomItem*>(index.internalPointer());

    QDomNode node = item->node();
    QDomElement element = node.toElement();
    QDomNamedNodeMap attributeMap = node.attributes();

    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column())
        {
        case 0:
        {
            QString label = element.attribute("label");
            if (label.isEmpty() && (element.tagName() == "object" || element.tagName() == "eNelements"))
                label = "object";
            return QString("%1:").arg(label);
        }
        case 1:
            return element.attribute("value");
        default:
            return QVariant();
        }
    case Qt::ForegroundRole:
    {
        QBrush brush;
        switch (index.column())
        {
        case 0:
            break;
        case 1:
            if (element.tagName() == "object")
                brush.setColor(Qt::blue);
            if (element.attribute("expand_to_fill") == "true")
                brush.setColor(Qt::gray);
            break;
        default:
            break;
        }
        return brush;
    }
    case Qt::FontRole:
    {
        QFont font;
        switch (index.column())
        {
        case 0:
            font.setBold(true);
            break;
        case 1:
            break;
        default:
            break;
        }
        return font;
    }
    default:
        return QVariant();
    }
}

Qt::ItemFlags LazyNutObject::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant LazyNutObject::headerData(int section, Qt::Orientation orientation,
                              int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Attribute");
        case 1:
            return tr("Value");
        default:
            return QVariant();
        }
    }

    return QVariant();
}

QModelIndex LazyNutObject::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    DomItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DomItem*>(parent.internalPointer());

    DomItem *childItem = parentItem->child(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex LazyNutObject::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    DomItem *childItem = static_cast<DomItem*>(child.internalPointer());
    DomItem *parentItem = childItem->parent();

    if (!parentItem || parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int LazyNutObject::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    DomItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DomItem*>(parent.internalPointer());

    return parentItem->node().childNodes().count();
}


void LazyNutObject::getObjFromDescriptionIndex(const QModelIndex &index)
{
    DomItem * item = static_cast<DomItem*>(index.internalPointer());
    if (item->node().nodeName() == "object" && index.column() == 1)
        emit objectRequested(data(index,Qt::DisplayRole).toString());
}



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
    // name, type, subtype
    return 3;
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
            case 2:
                return "subtype";
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
                return _objHash->value(_objHash->keys().at(index.row()))->name();
            case 1:
                return _objHash->value(_objHash->keys().at(index.row()))->type();
            case 2:
                return _objHash->value(_objHash->keys().at(index.row()))->subtype();
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

