#ifndef LAZYNUTOBJECTMODEL_H
#define LAZYNUTOBJECTMODEL_H

#include <QAbstractItemModel>
#include "lazynutobject.h"


class DomItem;
class QDomDocument;


// this class is based on the Qt simple DOM model example:
// http://doc.qt.io/qt-5/qtwidgets-itemviews-simpledommodel-example.html
class LazyNutObjectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    LazyNutObjectModel(QDomDocument *xmlDescription, QObject *parent = 0);
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
    QString name();
    QString type();

public slots:
    void removeDescription(QString descName);
    void updateDescription(QDomDocument* domDoc);
    void clearDescription();
    void pokeAdditionalDescription(const QModelIndex&,QDomDocument*);

signals:
    void objectRequested(QString);

private slots:
    void setDescription(QString name, QString type, QDomDocument* domDoc);
    void setDescription(QDomDocument* domDoc);
    void sendObjectRequested(const QModelIndex &index);

private:
    DomItem *rootItem;
};

#endif // LAZYNUTOBJECTMODEL_H
