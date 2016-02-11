#ifndef XMLMODEL_H
#define XMLMODEL_H

#include <QAbstractItemModel>
#include <QSharedPointer>

class DomItem;
class QDomDocument;
class QDomNode;


// this class is based on the Qt simple DOM model example:
// http://doc.qt.io/qt-5/qtwidgets-itemviews-simpledommodel-example.html
class XMLModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    XMLModel(QSharedPointer<QDomDocument> domDoc, QObject *parent = 0);
    ~XMLModel();
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;


private:
    DomItem *rootItem;
    QDomNode *rootNode;
};

#endif // XMLMODEL_H
