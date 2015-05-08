#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QStringList>

class TreeItem;

class TreeModel : public QAbstractItemModel
{
    // TreeModel implements a QAbstractItemModel tree based on TreeItem.
    // It is used to represent a LazyNutObj taxonomy, where each object is a leaf
    // and its genealogy line is object/type/subtype.
    // When used as taxonomy TreeModel has only one column.
    // In general the number of columns is determined by the number of headers
    // passed to the constructor.

    // TreeModel exposes a getGenealogy() slot that recursively computes
    // the genealogy line at any point in the tree.
    // The result is sent by the signal sendGenealogy.


    // This class is largely inspired by the Editable Tree Model Example
    // http://qt-project.org/doc/qt-4.8/itemviews-editabletreemodel.html

    Q_OBJECT

public:
    TreeModel(QStringList headers, QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                       const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                      int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation,
                            const QVariant &value, int role = Qt::EditRole);
    bool insertRows(int position, int rows,
                         const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows,
                         const QModelIndex &parent = QModelIndex());
    bool appendValue(const QVariant &value, const QModelIndex &parent = QModelIndex(),
                     int column = 0, int role = Qt::EditRole);
    bool appendSubtree(const TreeItem * subtree, const QModelIndex &parent = QModelIndex());
    TreeItem* getRootItem();

public slots:
    QList<QVariant> getGenealogy(const QModelIndex &index);

signals:
    //void recursiveGetGenealogy(const QModelIndex &index) const;
    void sendGenealogy(QList<QVariant>) const;


private:
    void recursiveGetGenealogy(const QModelIndex &index);

    //void setupModelData(const QStringList &lines, TreeItem *parent);
    TreeItem *getItem(const QModelIndex &index) const;
    TreeItem *rootItem;
    QList<QVariant> genealogy;
};

 #endif
