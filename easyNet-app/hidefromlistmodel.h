#ifndef HIDEFROMLISTMODEL_H
#define HIDEFROMLISTMODEL_H

#include <QSortFilterProxyModel>

class HideFromListModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    HideFromListModel(QObject *parent = Q_NULLPTR);
    void setRelevantColumn(int column) {relevantColumn = column;}

public slots:
    bool hideItem(QVariant item);
    bool showItem(QVariant item);
    bool showAll();

protected:
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const Q_DECL_OVERRIDE;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const Q_DECL_OVERRIDE;

private:
    QVariantList hiddenItems;
    int relevantColumn;
};

#endif // HIDEFROMLISTMODEL_H
