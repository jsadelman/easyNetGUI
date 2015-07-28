#ifndef OBJECTCATALOGUEFILTER_H
#define OBJECTCATALOGUEFILTER_H

#include <QSortFilterProxyModel>
#include "objectcatalogue.h"
class QDomDocument;

class ObjectCatalogueFilter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ObjectCatalogueFilter(QObject * parent = 0);
    bool isAllValid();

public slots:
    void setNoFilter() {setFilterRegExp(QString());}
    void setName(QString txt);
    void setNameList(QStringList list);
    void setType(QString txt);
    void setTypeList(QStringList list);

signals:
    void objectCreated(QString, QString, QDomDocument*);
    void objectDestroyed(QString name);
    void objectModified(QString name);

private slots:
    void sendObjectCreated(QModelIndex parent, int first, int last);
    void sendObjectDestroyed(QModelIndex parent, int first, int last);
    void sendObjectModified(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles);

private:
    void setList(QStringList list);
};

#endif // OBJECTCATALOGUEFILTER_H
