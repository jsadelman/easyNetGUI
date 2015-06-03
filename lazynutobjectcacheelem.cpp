#include "lazynutobjectcacheelem.h"

LazyNutObjectCacheElem::LazyNutObjectCacheElem()
{
    init();
}

LazyNutObjectCacheElem::LazyNutObjectCacheElem(const QString &name, const QString &type)
    : name(name), type(type)
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

