#ifndef LAZYNUTOBJECTCACHEELEM_H
#define LAZYNUTOBJECTCACHEELEM_H

#include <QString>
class QDomDocument;

struct LazyNutObjectCacheElem
{
public:
    LazyNutObjectCacheElem();
    LazyNutObjectCacheElem(const QString& name, const QString& type, const QString& subtype=QString());
    ~LazyNutObjectCacheElem();
    void init();
    QDomDocument* domDoc;
    QString name;
    QString type;
    QString subtype;
    bool invalid;
    bool pending;
};

#endif // LAZYNUTOBJECTCACHEELEM_H
