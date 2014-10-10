/*
     treeitem.cpp

     A container for items of data supplied by the simple tree model.
 */

 #include <QStringList>

 #include "treeitem.h"

 TreeItem::TreeItem(const QVector<QVariant> &itemData, TreeItem *parentItem):
     parentItem(parentItem), itemData(itemData)
 {
 }

 TreeItem::~TreeItem()
 {
     qDeleteAll(childItems);
 }

 void TreeItem::appendChild(TreeItem *item)
 {
     childItems.append(item);
 }

/* void TreeItem::removeLastChild()
 {
     if (!childItems.isEmpty())
     {
         childItems.removeLast();
     }
 }*/

 TreeItem *TreeItem::child(int row) const
 {
     return childItems.value(row);
 }

 const QList<TreeItem *> &TreeItem::children() const
 {
     return childItems;
 }

 int TreeItem::childRowNumber() const
  {
     // returns the position of this item in the children list of its parent
      if (parentItem)
          return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

      return 0;
  }


 TreeItem *TreeItem::lastChild()
 {
     return childItems.last();
 }

 int TreeItem::childCount() const
 {
     return childItems.count();
 }

 int TreeItem::columnCount() const
 {
     return itemData.count();
 }

 QVariant TreeItem::data(int column) const
 {
     return itemData.value(column);
 }

 bool TreeItem::insertChildren(int position, int count, int columns)
 {
     // creates count empty items and inserts them from position
     // in the list of children of this item.
     // Each empty item will have columns data slots in its itemData vector.
      if (position < 0 || position > childItems.size())
          return false;

      for (int row = 0; row < count; ++row) {
          QVector<QVariant> data(columns);
          TreeItem *item = new TreeItem(data, this);
          childItems.insert(position, item);
      }

      return true;
 }

 bool TreeItem::removeChildren(int position, int count)
 {
     if (position < 0 || position + count > childItems.size())
         return false;

     for (int row = 0; row < count; ++row)
         delete childItems.takeAt(position);

     return true;
 }

 bool TreeItem::removeLastChild()
 {
     return removeChildren(childItems.size()-1,1);
 }

 bool TreeItem::setData(int column, const QVariant &value)
{
      if (column < 0 || column >= itemData.size())
          return false;

      itemData[column] = value;
      return true;
}

 TreeItem *TreeItem::parent()
 {
     return parentItem;
 }

 void TreeItem::setParent(TreeItem* parent)
 {
     parentItem = parent;
 }

 /*int TreeItem::row() const
 {  // duplicates childRowNumber
     if (parentItem)
         return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

     return 0;
 }*/

 void TreeItem::print(QString &s, unsigned int depth) const
 {
     // recursive print, the QString s contains the result.
     s += indent(depth) + itemData[0].toString() + " : " + itemData[1].toString() + "\n";
     for (int ci = 0; ci < childItems.length(); ++ci)
     {
         childItems[ci]->print(s,depth+1);
     }
 }
