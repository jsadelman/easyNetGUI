/*
     treemodel.cpp

     Provides a simple tree model to show how to create and use hierarchical
     models.
 */

 #include <QtGui>

 #include "treeitem.h"
 #include "treemodel.h"

 TreeModel::TreeModel(QStringList headers, QObject *parent)
     : QAbstractItemModel(parent)
 {
     QVector<QVariant> rootData;
     foreach(QString header, headers)
         rootData << QVariant(header);

     rootItem = new TreeItem(rootData);
     genealogy.clear();
 }

 TreeModel::~TreeModel()
 {
     delete rootItem;
 }

 int TreeModel::columnCount(const QModelIndex & /*parent*/) const
 {
         return rootItem->columnCount();
 }

 QVariant TreeModel::data(const QModelIndex &index, int role) const
 {
     if (!index.isValid())
         return QVariant();

     if (role != Qt::DisplayRole && role != Qt::EditRole)
         return QVariant();

     TreeItem *item = getItem(index);

     return item->data(index.column());
 }

 Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
 {
     if (!index.isValid())
         return 0;

     return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
 }

 TreeItem *TreeModel::getItem(const QModelIndex &index) const
  {
      if (index.isValid()) {
          TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
          if (item) return item;
      }
      return rootItem;
  }

 QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
         return rootItem->data(section);

     return QVariant();
 }

 QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent)
             const
 {
     if (parent.isValid() && parent.column() != 0)
             return QModelIndex();

     TreeItem *parentItem = getItem(parent);

     TreeItem *childItem = parentItem->child(row);
     if (childItem)
         return createIndex(row, column, childItem);
     else
         return QModelIndex();
 }

 bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
  {
      TreeItem *parentItem = getItem(parent);
      bool success;

      beginInsertRows(parent, position, position + rows - 1);
      success = parentItem->insertChildren(position, rows, rootItem->columnCount());
      endInsertRows();

      return success;
  }


 QModelIndex TreeModel::parent(const QModelIndex &index) const
 {
     if (!index.isValid())
         return QModelIndex();

     TreeItem *childItem = getItem(index);
     TreeItem *parentItem = childItem->parent();

     if (parentItem == rootItem)
         return QModelIndex();

     return createIndex(parentItem->childRowNumber(), 0, parentItem);
 }

 bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
 {
     TreeItem *parentItem = getItem(parent);
     bool success = true;

     beginRemoveRows(parent, position, position + rows - 1);
     success = parentItem->removeChildren(position, rows);
     endRemoveRows();

     return success;
 }

 int TreeModel::rowCount(const QModelIndex &parent) const
 {
     TreeItem *parentItem = getItem(parent);

     return parentItem->childCount();
     /*TreeItem *parentItem;
     if (parent.column() > 0)
         return 0;

     if (!parent.isValid())
         parentItem = rootItem;
     else
         parentItem = static_cast<TreeItem*>(parent.internalPointer());

     return parentItem->childCount();*/
 }

 bool TreeModel::setData(const QModelIndex &index, const QVariant &value,
                          int role)
  {
      if (role != Qt::EditRole)
          return false;

      TreeItem *item = getItem(index);
      bool result = item->setData(index.column(), value);

      if (result)
          emit dataChanged(index, index);

      return result;
  }

  bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                                const QVariant &value, int role)
  {
      if (role != Qt::EditRole || orientation != Qt::Horizontal)
          return false;

      bool result = rootItem->setData(section, value);

      if (result)
          emit headerDataChanged(orientation, section, section);

      return result;
  }

  bool TreeModel::appendValue(const QVariant &value, const QModelIndex &parent, int column, int role)
  {
      if (!insertRows(rowCount(parent),1,parent))
      {
          qDebug() << "no insertRow";
          return false;
      }
      if (!setData(index(rowCount(parent)-1,column,parent),value,role))
      {
          qDebug() << "no setData";
          return false;
      }
      return true;
  }

  bool TreeModel::appendSubtree(const TreeItem * subtree, const QModelIndex &parent)
  {
      //subtree->setParent(getItem(parent));
      if (!insertRows(rowCount(parent),1,parent))
      {
          qDebug() << "no insertRow";
          return false;
      }
      for (int col = 0; col < columnCount(); ++col)
      {
          if (!setData(index(rowCount(parent)-1,col,parent),subtree->data(col)))
          {
              qDebug() << "no setData";
              return false;
          }

      }
      QModelIndex index_subtree = index(rowCount(parent)-1,0,parent);
      for (int ci = 0; ci < subtree->childCount(); ++ci)
      {
          if (!appendSubtree(subtree->child(ci),index_subtree))
              return false;

      }
      return true;
  }


  QList<QVariant> TreeModel::getGenealogy(const QModelIndex &index)
  {
      recursiveGetGenealogy(index);
      emit sendGenealogy(genealogy);
      return genealogy;
  }

  void TreeModel::recursiveGetGenealogy(const QModelIndex &index)
  {
      if (!index.isValid())
      {
          genealogy.clear();
          return;
      }
      recursiveGetGenealogy(index.parent());
      genealogy << data(index,Qt::DisplayRole);
      return;
  }


  /* debug purpose */
  TreeItem* TreeModel::getRootItem()
  {
      return rootItem;
  }

