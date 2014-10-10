#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QListIterator>
#include <QVariant>
#include <QVector>

 class TreeItem
 {
     // TreeItem implements a tree structure. Each item:
     // - contains data in QVector<QVariant> itemData
     // - points to its parent in TreeItem *parentItem
     // - points to its children in QList<TreeItem*> childItems
     // Member functions use words row and column to suggest
     // correspondence with QAbstractItemModel.

     // This class is largely inspired by the Editable Tree Model Example
     // http://qt-project.org/doc/qt-4.8/itemviews-editabletreemodel.html

 public:
     TreeItem(const QVector<QVariant> &itemData, TreeItem *parentItem = 0);
     ~TreeItem();

     void appendChild(TreeItem *child);
     bool insertChildren(int position, int count, int columns);
     bool removeChildren(int position, int count);
     bool removeLastChild();
     TreeItem *lastChild();

     TreeItem *child(int row) const;
     const QList<TreeItem*>& children() const; // maybe better use a pointer
     int childCount() const;
     int childRowNumber() const;
     bool setData(int column, const QVariant &value);
     int columnCount() const; // todo
     QVariant data(int column) const;
     int row() const;
     TreeItem *parent();
     void setParent(TreeItem* parent);
     // print functions for debug purpose
     void print(QString &s, unsigned int depth=0) const;
     static inline QString indent(unsigned int d)
     {
         return QString(d * 4, ' ');
     }

 private:
     QList<TreeItem*> childItems;
     QVector<QVariant> itemData;
     TreeItem *parentItem;
 };

 #endif
