#ifndef DESCRIPTIONUPDATER_H
#define DESCRIPTIONUPDATER_H

#include <QObject>
#include <QModelIndex>

class QSortFilterProxyModel;
class ObjectCatalogue;

class DescriptionUpdater : public QObject
{
    Q_OBJECT
public:
    explicit DescriptionUpdater(QString updaterName, QObject *parent = 0);
    void setProxyModel(QSortFilterProxyModel *proxy);

signals:

private slots:
    void requestDescriptions(QModelIndex top, QModelIndex bottom);

private:
    void requestDescription(QString updaterName);

    QSortFilterProxyModel *proxyModel;
    ObjectCatalogue *objectCatalogue;
    QString updaterName;
};

#endif // DESCRIPTIONUPDATER_H
