#ifndef PROXYMODELEXTRAROWS_H
#define PROXYMODELEXTRAROWS_H

#include <QSortFilterProxyModel>
#include <QIdentityProxyModel>

class ProxyModelExtraRows : public QIdentityProxyModel
{
    // http://stackoverflow.com/questions/3730117/qsortfilterproxymodel-returning-artificial-row
    Q_OBJECT
public:
    ProxyModelExtraRows(QObject * parent = 0);
    void setHeadingItems(QStringList items);
    void setTrailingtems(QStringList items);
    int	rowCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const Q_DECL_OVERRIDE;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
//    QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;

private:
    QStringList headingItems;
    QStringList trailingItems;
    int headingCount;
    int trailingCount;
};

#endif // PROXYMODELEXTRAROWS_H
