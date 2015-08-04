#include "domitem.h"
#include <QDebug>

DomItem::DomItem(QDomNode *node, int row, DomItem *parent)
{
    if (node)
        domNode = *node;
    // Record the item's location within its parent.
    rowNumber = row;
    parentItem = parent;
}

void DomItem::assign(QDomNode*node)
{
   childItems.clear();
   domNode=*node;
}

DomItem::~DomItem()
{
    QHash<int,DomItem*>::iterator it;
    for (it = childItems.begin(); it != childItems.end(); ++it)
        delete it.value();
}

QDomNode DomItem::node() const
{
    return domNode;
}

DomItem *DomItem::parent()
{
    return parentItem;
}

DomItem *DomItem::child(int i)
{
    if (childItems.contains(i))
        return childItems[i];

    if (i >= 0 && i < domNode.childNodes().count()) {
        QDomNode childNode = domNode.childNodes().item(i);
        DomItem *childItem = new DomItem(&childNode, i, this);
        childItems[i] = childItem;
        return childItem;
    }
    return 0;
}

DomItem *DomItem::nonTextChild(int i)
{
    if (childItems.contains(i))
        return childItems[i];
    if (i < 0 || i >= domNode.childNodes().count())
        return 0;

    int count = -1;
    for (int j = 0; j < domNode.childNodes().count(); ++j)
    {
        QDomNode childNode = domNode.childNodes().item(j);
        if (childNode.isElement())
            ++count;
        if (count == i)
        {
            DomItem *childItem = new DomItem(&childNode, i, this);
            childItems[i] = childItem;
            return childItem;
        }
    }
    return 0;
}

int DomItem::nonTextRowCount()
{
    int count = 0;
    QDomNode childNode = domNode.firstChild();
    while (!childNode.isNull())
    {
        if (childNode.isElement())
            ++count;
        childNode = childNode.nextSibling();
    }
    return count;
}

QString DomItem::text()
{
    // return the first text you find, assume only one text at child level
    QDomNode node = domNode.firstChild();
    while (!node.isNull())
    {
        if (node.isText())
            return node.nodeValue();
        node = node.nextSibling();
    }
    return QString();
}



int DomItem::row()
{
    return rowNumber;
}
