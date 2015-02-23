
#ifndef DOMMODEL_H
#define DOMMODEL_H

#include <QAbstractItemModel>
#include <QDomDocument>
#include <QModelIndex>
#include <QVariant>

class DomItem;

// this class is based on the following example:
// http://doc.qt.digia.com/4.6/itemviews-simpledommodel.html

class LazyNutObject : public QAbstractItemModel
{
    Q_OBJECT

public:
    LazyNutObject(QDomDocument *doc, QObject *parent = 0);
    ~LazyNutObject();

    QVariant data(const QModelIndex &index, int role) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    inline QString name() {return _name;}
    inline QString type() {return _type;}
    inline QString subtype() {return _subtype;}
    QString getValue(QString label);


signals:
    void objectRequested(QString);

private slots:
    void getObjFromDescriptionIndex(const QModelIndex &index);

private:
    void initProperties();

    QDomDocument *domDoc;
    DomItem *rootItem;
    QString _name;
    QString _type;
    QString _subtype;

};


#endif
