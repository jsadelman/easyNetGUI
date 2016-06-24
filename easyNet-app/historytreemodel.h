#ifndef HISTORYTREEMODEL_H
#define HISTORYTREEMODEL_H

#include "treemodel.h"

class ObjectCacheFilter;
class ObjectUpdater;
class QDomDocument;

struct CheckRecord
{
    CheckRecord() : name(""), prettyName(""), checked(false) {}
    QString name;
    QString prettyName;
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
    bool containsView(QString view);
    bool isInView(QString view);

    bool appendTrial(QString trial);
    bool removeTrial(QString trial);
    bool appendView(QString view, QString trial, bool inView=false);
    bool removeView(QString view, QString trial="");
    bool setInView(QString view, QString trial, bool inView);
    bool setInView(QString view, bool inView);

    QModelIndex trialIndex(QString trial);
    QModelIndex viewIndex(QString view, QString trial);
    QModelIndex viewIndex(QString view);
    QString trial(QString view);

private slots:
    void updatePrettyName(QDomDocument* description, QString name);

private:
    ObjectCacheFilter *filter;
    ObjectUpdater *updater;

};

#endif // HISTORYTREEMODEL_H
