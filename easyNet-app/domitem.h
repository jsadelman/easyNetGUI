
#ifndef DOMITEM_H
#define DOMITEM_H

#include <QDomNode>
#include <QHash>

// this class is based on the Qt simple DOM model example:
// http://doc.qt.io/qt-5/qtwidgets-itemviews-simpledommodel-example.html

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
