#ifndef LAZYNUTOBJ_H
#define LAZYNUTOBJ_H

#include <QPair>
#include <QString>
#include <QVariant>
#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QHash>




// A centralised LazyNut object catalogue is implemented by a hash where
// key = object name, value = pointer to object.
// Objects refer to each other,
// e.g. a layer has a property listing incoming connections, which are also objects.
// This is conceptually a graph. Its implementation is based on the hash itself,
// i.e. whenever an object property value is an other object's name,
// the corresponding object can be retrievd by using it as key in the hash itself.

class LazyNutObject;
typedef QHash<QString,LazyNutObject*> LazyNutObjectCatalogue;




//class LazyNutObjTableModel : public QAbstractTableModel
// {
//    // LazyNutObjTableModel represents a table where each row refers to a lazyNut object,
//    // and three columns contain objects name, type and subtype.
//    // It is used as base model for LazyNutObjTableProxyModel, which selects only
//    // the name column and a subset of objects.
//    // Its internal representation is a LazyNutObjCatalogue (_objHash).

//    // Public slots sendBeginResetModel and sendEndResetModel are exposed in order to
//    // trigger protected members beginResetModel and endResetModel from external signals.

//     Q_OBJECT

// public:
//     LazyNutObjTableModel(LazyNutObjectCatalogue *objHash=nullptr, QObject *parent = 0);
//     int rowCount(const QModelIndex &parent = QModelIndex()) const;
//     int columnCount(const QModelIndex &parent = QModelIndex()) const;


//public slots:
//     void sendBeginResetModel();
//     void sendEndResetModel();

// private:
//    LazyNutObjectCatalogue *_objHash;
// };


class LazyNutObjectListModel : public QAbstractListModel
{
    // a list of available objects,
    // going to be substituted by smarter and lazyNut model-centered structure
    Q_OBJECT

public:
    LazyNutObjectListModel(LazyNutObjectCatalogue *objHash = 0, QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

signals:
    void objectRequested(QString);

public slots:
     void sendBeginResetModel();
     void sendEndResetModel();
     void getObjFromListIndex(const QModelIndex &index);

 private:
    LazyNutObjectCatalogue *_objHash;
};

//class LazyNutObjTableProxyModel : public QSortFilterProxyModel
// {
//    // LazyNutObjTableProxyModel implements a filter on a LazyNutObjTableModel object.
//    // It shows only the rows (lazyNut objects) that belong to a given object genealogy,
//    // e.g. only objects of type layer, or only of subtype iac_layer.
//    // A genealogy is a list like {"object", "layer", "iac_layer"}, or
//    //  {"object", "layer"} or {"object"}.
//    // The filtering criterion is stored in the QMap _columnPatterns,
//    // where keys are column indices in a LazyNutObjTableModel and values are
//    // filtering strings, e.g. "iac_layer".

//     Q_OBJECT

// public:
//     LazyNutObjTableProxyModel(QObject *parent = 0);
//     void setFilterKeyColumns(const QList<int> &filterColumns);
//     void addFilterFixedString(int column, const QString &pattern);

// public slots:
//     void setFilterFromGenealogy(const QList<QVariant> genealogy);
//     void mapIndexToSourceModel(const QModelIndex & index);

// signals:
//     void getObj(const QModelIndex & index);

//protected:
//     bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

//private:
//            QMap<int, QString> _columnPatterns;

//};

#endif // LAZYNUTOBJ_H
