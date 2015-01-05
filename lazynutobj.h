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



class LazyNutObj
{
    // LazyNutObj contains information on one LazyNut object, e.g. a layer.
    // It contains the information retrieved by issuing a description query to lazyNut.
    // Each object is represented by a list of pairs of property key-value (_properties).
    // LazyNut objects have at least three properties: name, type and subtype.
    // Further, an arbitrary number of properties is allowed.
    // A property value (a QVariant) is usually either a QString or a QStringList,
    // the latter case being exposed by rowsWithStringList to be used by views.
    // To speed up access, a hash (_keyAt) is used to map property keys to position in the list.

public:
    LazyNutObj() {}
    void appendProperty(const QString& key, const QVariant& value);
    QVariant getValue(const QString& key);
    QVariant getValue(int i);
    QString getKey(int i);
    QPair<QString, QVariant> getProperty(int i);
    int rowCount();
    QList<int> rowsWithStringList();
    QString& name();
    QString& type();
    QString& subtype();

private:
    QList<QPair<QString,QVariant> > _properties;
    QHash<QString,int> _keyAt;
    QString _name;
    QString _type;
    QString _subtype;

};

// A centralised LazyNut object catalogue is implemented by a hash where
// key = object name, value = pointer to object.
// Objects refer to each other,
// e.g. a layer has a property listing incoming connections, which are also objects.
// This is conceptually a graph. Its implementation is based on the hash itself,
// i.e. whenever an object property value is an other object's name,
// the corresponding object can be retrievd by using it as key in the hash itself.
typedef QHash<QString,LazyNutObj*> LazyNutObjCatalogue;


class LazyNutObjModel : public QAbstractListModel
 {
    // LazyNutObjModel represents a single lazyNut object.
    // Its internal representation is a LazyNutObj instance.
    // A list is sufficient, since an object is a set of properties,
    // each one being a name-value pair.
    // Names are set as vertical header, values are the data.

    // LazyNutObjModel makes use of the central catalogue (_objHash)
    // in order to retrieve pointers to objects referred to by a QModelIndex
    // belonging either to this class or to the LazyNutObjTableModel class.
    // This functionality is necessary to navigate the object explorer GUI.
    // When the user clicks on an object name in a view based on this model class,
    // the getObjFromDescriptionIndex slot should be called. Two implementations
    // are available, one for property values that contain the object name (QString),
    // another for property values that contain a list of names (QStringList), where
    // an extra position index is required.
    // When the user clicks on an object name in a view based on LazyNutObjTableModel,
    // the getObjFromCatalogueIndex slot should be called.

     Q_OBJECT

 public:
     LazyNutObjModel(LazyNutObj *obj, LazyNutObjCatalogue *objHash=nullptr,
                     QObject *parent = 0):
         QAbstractListModel(parent), _obj(obj), _objHash(objHash) {}

     int rowCount(const QModelIndex &parent = QModelIndex()) const;
     QVariant data(const QModelIndex &index, int role) const;
     QVariant headerData(int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;

     bool isNameInCatalogue(const QString& name);

public slots:

    void getObjFromDescriptionIndex(const QModelIndex & index);
    void getObjFromDescriptionIndex(const QModelIndex & index, int objIndex);
    void getObjFromCatalogueIndex(const QModelIndex & index);

signals:
    void showObj(LazyNutObj * obj, LazyNutObjCatalogue* objHash);

 private:

    LazyNutObj *_obj;
    LazyNutObjCatalogue *_objHash;
 };

class LazyNutObjTableModel : public QAbstractTableModel
 {
    // LazyNutObjTableModel represents a table where each row refers to a lazyNut object,
    // and three columns contain objects name, type and subtype.
    // It is used as base model for LazyNutObjTableProxyModel, which selects only
    // the name column and a subset of objects.
    // Its internal representation is a LazyNutObjCatalogue (_objHash).

    // Public slots sendBeginResetModel and sendEndResetModel are exposed in order to
    // trigger protected members beginResetModel and endResetModel from external signals.

     Q_OBJECT

 public:
     LazyNutObjTableModel(LazyNutObjCatalogue *objHash=nullptr, QObject *parent = 0);
     int rowCount(const QModelIndex &parent = QModelIndex()) const;
     int columnCount(const QModelIndex &parent = QModelIndex()) const;
     QVariant data(const QModelIndex &index, int role) const;
     QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public slots:
     void sendBeginResetModel();
     void sendEndResetModel();

 private:
    LazyNutObjCatalogue *_objHash;
 };

class LazyNutObjTableProxyModel : public QSortFilterProxyModel
 {
    // LazyNutObjTableProxyModel implements a filter on a LazyNutObjTableModel object.
    // It shows only the rows (lazyNut objects) that belong to a given object genealogy,
    // e.g. only objects of type layer, or only of subtype iac_layer.
    // A genealogy is a list like {"object", "layer", "iac_layer"}, or
    //  {"object", "layer"} or {"object"}.
    // The filtering criterion is stored in the QMap _columnPatterns,
    // where keys are column indices in a LazyNutObjTableModel and values are
    // filtering strings, e.g. "iac_layer".

     Q_OBJECT

 public:
     LazyNutObjTableProxyModel(QObject *parent = 0);
     void setFilterKeyColumns(const QList<int> &filterColumns);
     void addFilterFixedString(int column, const QString &pattern);

 public slots:
     void setFilterFromGenealogy(const QList<QVariant> genealogy);
     void mapIndexToSourceModel(const QModelIndex & index);

 signals:
     void getObj(const QModelIndex & index);

protected:
     bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
            QMap<int, QString> _columnPatterns;

};

#endif // LAZYNUTOBJ_H
