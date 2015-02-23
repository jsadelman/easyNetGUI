
#include <QtGui>
#include <QtXml>

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
        QString value = objectNode.firstChild().nodeValue().simplified();
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
            return objectNode.firstChild().nodeValue().simplified();
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
            //return element.attribute("value");
            return item->text();
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

//    DomItem *childItem = parentItem->child(row);
    DomItem *childItem = parentItem->nonTextChild(row);

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

//    return parentItem->node().childNodes().count();
    return parentItem->nonTextRowCount();
}


void LazyNutObject::getObjFromDescriptionIndex(const QModelIndex &index)
{
    DomItem * item = static_cast<DomItem*>(index.internalPointer());
    if (item->node().nodeName() == "object" && index.column() == 1)
        emit objectRequested(item->text().simplified());
}
