#ifndef SELECTFROMLISTMODEL_H
#define SELECTFROMLISTMODEL_H

//#include <QIdentityProxyModel>
#include <QAbstractTableModel>

class SelectFromListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SelectFromListModel(QObject * parent = 0);
    void setRelevantColumn(int column) {relevantColumn = column;}
    int rowCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    void setSourceModel(QAbstractItemModel * model);
    QModelIndexList match(const QModelIndex & start, int role, const QVariant & value, int hits = 1,
                          Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchStartsWith | Qt::MatchWrap )) const Q_DECL_OVERRIDE;

private slots:
    void updateNewRows(QModelIndex parent,int first, int last);
    void updateRemovedRows(QModelIndex parent,int first, int last);
    void updateRows(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int> &roles);

private:
    QAbstractItemModel *sourceModel;
    int relevantColumn;
    QHash<QString, bool> isSelected;
};

#endif // SELECTFROMLISTMODEL_H
