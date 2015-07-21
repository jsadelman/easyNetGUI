
#ifndef DOMITEM_H
#define DOMITEM_H

#include <QDomNode>
#include <QHash>

// this class is based on the following example:
// http://doc.qt.digia.com/4.6/itemviews-simpledommodel.html

class DomItem
{
public:
    DomItem(QDomNode *node, int row, DomItem *parent = 0);
    ~DomItem();
    DomItem *child(int i);
    DomItem *nonTextChild(int i);
    int     nonTextRowCount();
    QString text();
    DomItem *parent();
    QDomNode node() const;
    int row();
    void assign(QDomNode*node);

private:
    QDomNode domNode;
    QHash<int,DomItem*> childItems;
    DomItem *parentItem;
    int rowNumber;
};

#endif
