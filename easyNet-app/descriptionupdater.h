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
    void requestDescription(QString name);

signals:
    void descriptionUpdated(QDomDocument*);

protected slots:
    void goToSleep();
    void wakeUpUpdate();
private slots:
    void requestDescriptions(QModelIndex top, QModelIndex bottom);
    void requestDescriptions(QModelIndex parent, int first, int last);
    void errorHandler(QString cmd, QStringList errorList);

private:
    void requestDescriptions(int first, int last);
    QStringList getObjectNames(int first, int last);

    QSortFilterProxyModel *proxyModel;
    ObjectCatalogue *objectCatalogue;
};

#endif // DESCRIPTIONUPDATER_H
