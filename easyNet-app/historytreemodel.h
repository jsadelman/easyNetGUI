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
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value,
                    int role = Qt::EditRole) Q_DECL_OVERRIDE;
    bool containsTrial(QString trial);
    bool containsView(QString view, QString trial);
    bool appendTrial(QString trial);
    bool removeTrial(QString trial);
    bool appendView(QString view, QString trial, bool inView=false);
    bool removeView(QString view, QString trial);
    bool setInView(QString view, QString trial, bool inView);
    bool isInView(QString view, QString trial);
    QModelIndex trialIndex(QString trial);
    QModelIndex viewIndex(QString view, QString trial);
    QModelIndex viewIndex(QString view);
};

#endif // HISTORYTREEMODEL_H
