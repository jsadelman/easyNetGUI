#ifndef HISTORYTREEMODEL_H
#define HISTORYTREEMODEL_H

#include "treemodel.h"

struct CheckRecord
{
    QString text;
    bool checked;
};
Q_DECLARE_METATYPE(CheckRecord)

class HistoryTreeModel : public TreeModel
{
    Q_OBJECT
public:
    explicit HistoryTreeModel(QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
//    QVariant headerData(int section, Qt::Orientation orientation,
//                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
//    QModelIndex index(int row, int column,
//                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
//    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
//    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value,
                    int role = Qt::EditRole) Q_DECL_OVERRIDE;
//    virtual bool appendValue(const QVariant &value, const QModelIndex &parent = QModelIndex(),
//                     int column = 0, int role = Qt::EditRole);
//    bool insertRows(int position, int rows,
//                    const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
//    bool removeRows(int position, int rows,
//                    const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
//    bool setTrialRunInfo(QString item, QSharedPointer<QDomDocument> info);
    bool containsTrial(QString trial);
    bool containsView(QString view, QString trial);
    bool appendTrial(QString trial);
    bool removeTrial(QString trial);
    bool appendView(QString view, QString trial, bool inView=false);
    bool removeView(QString view, QString trial);

signals:
    void checkDataChanged();

private:
    QModelIndex trialIndex(QString trial);
    QModelIndex viewIndex(QString view, QString trial);
};

#endif // HISTORYTREEMODEL_H
