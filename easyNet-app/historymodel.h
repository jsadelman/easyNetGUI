#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H


#include <QAbstractListModel>



// largely copied from ObjectCache

class CheckListModel: public QAbstractListModel
{
    Q_OBJECT
public:
    CheckListModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
//    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE;
//    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
    void clear();
    bool contains(const QString& name);
    bool isInViewer(const QString& name);
    QStringList selectedItems();

public slots:
    bool create(const QString& name);
    bool destroy(const QString& name);

signals:
    void checkDataChanged();

private:
    struct HistoryRecord
    {
        QString name;
        bool checked;
    };
    int rowFromName(const QString &name);

    QList<HistoryRecord> m_history;

};

#endif // HISTORYMODEL_H
