#include "lazynutobjectcacheelem.h"

LazyNutObjectCacheElem::LazyNutObjectCacheElem()
    : domDoc(nullptr), invalid(true), pending(true)
{
}

LazyNutObjectCacheElem::~LazyNutObjectCacheElem()
{
    if (domDoc)
        delete domDoc;
}

