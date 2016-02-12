#include "xmlmodel.h"
#include "domitem.h"
#include "xmlelement.h"

#include <QFont>
#include <QBrush>
#include <QDebug>
#include <QDomNode>

XMLModel::XMLModel(QSharedPointer<QDomDocument> domDoc, QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new DomItem(domDoc, 0, 1);
}

XMLModel::~XMLModel()
{
}

QVariant XMLModel::data(const QModelIndex &index, int role) const
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
//            if (label.isEmpty() && (XMLelem.isObject() || XMLelem.isENelements()))
//                label = "object";

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

Qt::ItemFlags XMLModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (XMLelement(static_cast<DomItem*>(index.internalPointer())->node().toElement()).isCommand())
        flags |= Qt::ItemNeverHasChildren;

    return flags;
}

QVariant XMLModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QModelIndex XMLModel::index(int row, int column, const QModelIndex &parent) const
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

QModelIndex XMLModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    DomItem *childItem = static_cast<DomItem*>(child.internalPointer());
    DomItem *parentItem = childItem->parent();

    if (!parentItem || parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int XMLModel::rowCount(const QModelIndex &parent) const
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

int XMLModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

