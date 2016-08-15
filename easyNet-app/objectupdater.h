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
    Q_PROPERTY(bool dependencies READ dependencies WRITE setDependencies)
public:
    explicit ObjectUpdater(QObject *parent = 0);
    void setProxyModel(QSortFilterProxyModel *proxy);
    void setFilter(ObjectCacheFilter *filter);
    bool dependencies() {return m_dependencies;}
    void setDependencies(bool dependencies) {m_dependencies = dependencies;}


signals:
    void objectUpdated(QDomDocument*, QString);

public slots:
    void goToSleep();
    void wakeUpUpdate();
    void setCommand(QString command) {m_command = command;}
    void requestObject(QString name, QString command="");
private slots:
    void requestObjects(QModelIndex top, QModelIndex bottom);
    void requestObjects(QModelIndex parent, int first, int last);
    void errorHandler(QString cmd, QString error);

private:
    void requestObjects(int first, int last);
    QStringList getObjectNames(int first, int last);

    QSortFilterProxyModel *proxyModel;
    ObjectCache *objectCache;
    QString m_command;
    bool m_dependencies;
    bool m_awake;
};

#endif // DESCRIPTIONUPDATER_H
