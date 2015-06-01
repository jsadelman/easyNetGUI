#ifndef DESCRIPTIONUPDATER_H
#define DESCRIPTIONUPDATER_H

#include <QObject>
#include <QModelIndex>

class QSortFilterProxyModel;
class ObjectCatalogue;
class QDomDocument;

class DescriptionUpdater : public QObject
{
    Q_OBJECT
public:
    explicit DescriptionUpdater(QObject *parent = 0);
    void setProxyModel(QSortFilterProxyModel *proxy);

signals:
    void descriptionUpdated(QDomDocument*);

private slots:
    void requestDescriptions(QModelIndex top, QModelIndex bottom);
    void requestDescriptions(QModelIndex parent, int first, int last);
    void notifyDescriptionUpdated(QDomDocument* domDoc);

private:
    void requestDescriptions(int first, int last);
    QStringList getObjectNames(int first, int last);
    void requestDescription(QString name);

    QSortFilterProxyModel *proxyModel;
    ObjectCatalogue *objectCatalogue;
};

#endif // DESCRIPTIONUPDATER_H
