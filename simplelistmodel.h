#ifndef SIMPLELISTMODEL_H
#define SIMPLELISTMODEL_H

#include <QObject>
#include <QAbstractListModel>

class StringListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    StringListModel(QStringList list = QStringList(), QObject * parent = 0);
    int rowCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE;
    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
    bool appendStrings(QStringList txtList);
    bool appendString(QString txt);
    bool removeString(QString txt);
    bool removeStrings(QStringList txtList);

    QStringList getList() {return list;}
    bool contains(QString s) {return list.contains(s);}

public slots:
    bool updateList(QStringList newList);


private:
    QStringList list;
};

#endif // SIMPLELISTMODEL_H
