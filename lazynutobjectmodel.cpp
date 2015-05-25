#include <QFont>
#include <QBrush>

#include "lazynutobjectmodel.h"
#include "xmlelement.h"
#include "domitem.h"
#include "enumclasses.h"

LazyNutObjectModel::LazyNutObjectModel(QDomDocument *domDoc, QObject *parent)
    : LazyNutObject(domDoc), QAbstractItemModel(parent)
{
    rootItem = new DomItem(domDoc, 0);
    //    initProperties();
}

LazyNutObjectModel::LazyNutObjectModel(LazyNutObject *lno, QObject *parent)
    : LazyNutObject(*lno), QAbstractItemModel(parent)
{
    rootItem = new DomItem(domDoc, 0);
}

LazyNutObjectModel::~LazyNutObjectModel()
{
//    delete domDoc;
}

QVariant LazyNutObjectModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    DomItem *item = static_cast<DomItem*>(index.internalPointer());
    XMLelement XMLelem = XMLelement(item->node().toElement());
    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column())
        {
        case 0:
        {
            QString label = XMLelem.label();
            if (label.isEmpty() && (XMLelem.isObject() || XMLelem.isENelements()))
                label = "object";

            return QString("%1:").arg(label);
        }
        case 1:
        {
            if (XMLelem.isCommand())
                return XMLelem.command();

            else
                return XMLelem.value();
        }
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
        {
            if (XMLelem.isObject())
                brush.setColor(Qt::blue);

            break;
        }
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
    case ExpandToFillRole:
        switch (index.column())
        {
        case 0:
            return false;
        case 1:
        {
            if (XMLelem.isCommand() && XMLelem.attribute("expand_to_fill") == "true")
                return true;

            else
                return false;
        }
        default:
            return false;
        }
    default:
        return QVariant();
    }
}

Qt::ItemFlags LazyNutObjectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;;
    if (XMLelement(static_cast<DomItem*>(index.internalPointer())->node().toElement()).isCommand())
        flags |= Qt::ItemNeverHasChildren;

    return flags;
}

QVariant LazyNutObjectModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QModelIndex LazyNutObjectModel::index(int row, int column, const QModelIndex &parent) const
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

QModelIndex LazyNutObjectModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    DomItem *childItem = static_cast<DomItem*>(child.internalPointer());
    DomItem *parentItem = childItem->parent();

    if (!parentItem || parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int LazyNutObjectModel::rowCount(const QModelIndex &parent) const
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

int LazyNutObjectModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

void LazyNutObjectModel::getObjFromDescriptionIndex(const QModelIndex &index)
{
    DomItem * item = static_cast<DomItem*>(index.internalPointer());
    if (XMLelement(item->node().toElement()).isObject() && index.column() == 1)
        emit objectRequested(data(index,Qt::DisplayRole).toString());
}

//QString LazyNutObjectModel::getValue(const QString &label)
//{
//    return XMLelement(*domDoc)[label]();
//}

//void LazyNutObjectModel::initProperties()
//{
//    XMLelement XMLroot = XMLelement(*domDoc);
//    _name = XMLroot["this"]();
//    QString typeStr = XMLroot["type"]();
//    _type = typeStr.section('/',0,0);
//    _subtype = typeStr.section('/',1,1);
//}

