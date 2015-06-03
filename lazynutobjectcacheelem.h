#ifndef LAZYNUTOBJECTCACHEELEM_H
#define LAZYNUTOBJECTCACHEELEM_H

#include <QString>
class QDomDocument;

struct LazyNutObjectCacheElem
{
public:
    LazyNutObjectCacheElem();
    LazyNutObjectCacheElem(const QString& name, const QString& type);
    ~LazyNutObjectCacheElem();
    void init();
    QDomDocument* domDoc;
    QString name;
    QString type;
    bool invalid;
    bool pending;
};

#endif // LAZYNUTOBJECTCACHEELEM_H
