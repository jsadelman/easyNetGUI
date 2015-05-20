#include <utility>

#include "lazynutobject.h"
#include "xmlelement.h"



LazyNutObject::LazyNutObject(QDomDocument *domDoc)
    : domDoc(domDoc)
{
    initProperties();
}

// copy ctor: shallow copy domDoc but recreate (do not copy) const refs name, type, subtype
LazyNutObject::LazyNutObject(const LazyNutObject &other)
    : LazyNutObject(other.domDoc)
{
}

// http://en.cppreference.com/w/cpp/language/as_operator
LazyNutObject &LazyNutObject::operator=(LazyNutObject other)
{
    std::swap(other, *this);
    return *this;
}

LazyNutObject::~LazyNutObject()
{
    delete domDoc;
}


void LazyNutObject::initProperties()
{
    XMLelement XMLroot = XMLelement(*domDoc);
    _name = XMLroot["this"]();
    //QString typeStr = XMLroot["type"]();
    _type = XMLroot["type"]();
    _subtype = XMLroot["subtype"]();
}


QString LazyNutObject::getValue(const QString &label)
{
    return XMLelement(*domDoc)[label]();
}
