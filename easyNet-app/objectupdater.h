#ifndef DESCRIPTIONUPDATER_H
#define DESCRIPTIONUPDATER_H

#include <QObject>
#include <QModelIndex>

class QSortFilterProxyModel;
class ObjectCache;
class ObjectCacheFilter;
class QDomDocument;

class ObjectUpdater : public QObject
{
    Q_OBJECT
public:
    explicit ObjectUpdater(QObject *parent = 0);
    void setProxyModel(QSortFilterProxyModel *proxy);
    void setFilter(ObjectCacheFilter *filter);


signals:
    void objectUpdated(QDomDocument*, QString);

public slots:
    void goToSleep();
    void wakeUpUpdate();
    void setCommand(QString command) {m_command = command;}
    void requestObject(QString name);
private slots:
    void requestObjects(QModelIndex top, QModelIndex bottom);
    void requestObjects(QModelIndex parent, int first, int last);
    void errorHandler(QString cmd, QStringList errorList);

private:
    void requestObjects(int first, int last);
    QStringList getObjectNames(int first, int last);

    QSortFilterProxyModel *proxyModel;
    ObjectCache *objectCache;
    QString m_command;
};

#endif // DESCRIPTIONUPDATER_H
