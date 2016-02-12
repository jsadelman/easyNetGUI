#include "lazynutobjectcacheelem.h"

LazyNutObjectCacheElem::LazyNutObjectCacheElem()
{
    init();
}

LazyNutObjectCacheElem::LazyNutObjectCacheElem(const QString &name, const QString &type, const QString &subtype)
    : name(name), type(type), subtype(subtype)
{
    init();
}

LazyNutObjectCacheElem::~LazyNutObjectCacheElem()
{
    if (domDoc)
        delete domDoc;
}

void LazyNutObjectCacheElem::init()
{
    domDoc = nullptr;
    invalid = true;
    pending = true;
}

