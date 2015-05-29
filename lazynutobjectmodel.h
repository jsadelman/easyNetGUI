#ifndef LAZYNUTOBJECTMODEL_H
#define LAZYNUTOBJECTMODEL_H

#include <QAbstractItemModel>
#include "lazynutobject.h"


class DomItem;
class QDomDocument;


// this class is based on the following example:
// http://doc.qt.digia.com/4.6/itemviews-simpledommodel.html
class LazyNutObjectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    LazyNutObjectModel(QDomDocument *domDoc, QObject *parent = 0);
    LazyNutObjectModel(AsLazyNutObject *lno, QObject *parent = 0);
    ~LazyNutObjectModel();
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
//    inline QString name() {return _name;}
//    inline QString type() {return _type;}
//    inline QString subtype() {return _subtype;}
//    QString getValue(const QString& label);

signals:
    void objectRequested(QString);

private slots:
    void getObjFromDescriptionIndex(const QModelIndex &index);

private:
    void initProperties();

//    QDomDocument *domDoc;
    DomItem *rootItem;
//    QString _name;
//    QString _type;
//    QString _subtype;
};

#endif // LAZYNUTOBJECTMODEL_H
