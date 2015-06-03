#ifndef OBJECTCATALOGUEFILTER_H
#define OBJECTCATALOGUEFILTER_H

#include <QSortFilterProxyModel>

class ObjectCatalogue;
class QDomDocument;

class ObjectCatalogueFilter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ObjectCatalogueFilter(ObjectCatalogue *objectCatalogue, QObject * parent = 0);

public slots:
    void setNoFilter() {setFilterRegExp(QString());}
    void setName(QString txt);
    void setNameList(QStringList list);
    void setType(QString txt);
    void setTypeList(QStringList list);

signals:
    void objectCreated(QString, QString, QDomDocument*);
    void objectDestroyed(QString name);

private slots:
    void sendObjectCreated(QModelIndex parent, int first, int last);
    void sendObjectDestroyed(QModelIndex parent, int first, int last);

private:
    void setList(QStringList list);
    ObjectCatalogue *objectCatalogue;
};

#endif // OBJECTCATALOGUEFILTER_H
