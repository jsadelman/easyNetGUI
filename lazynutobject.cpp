#include <utility>
#include <QDebug>

#include "lazynutobject.h"
#include "xmlelement.h"


AsLazyNutObject::AsLazyNutObject(QDomDocument domDoc)
    : XMLelement(domDoc)
{
    if (label() != "this")
        qDebug () << "LazyNutObject: object description has no 'this' label";
    if (value().isEmpty())
        qDebug () << "LazyNutObject: object description has empty name";
    if ((*this)["type"]().isEmpty())
        qDebug () << "LazyNutObject: object description has empty type";
        // TODO: check type is admissible
}


//LazyNutObject::LazyNutObject(QDomDocument *domDoc)
//    : domDoc(domDoc)
//{
//    initProperties();
//}

// copy ctor: shallow copy domDoc but recreate (do not copy) const refs name, type, subtype
//LazyNutObject::LazyNutObject(const LazyNutObject &other)
//    : LazyNutObject(other.domDoc)
//{
//}

//// http://en.cppreference.com/w/cpp/language/as_operator
//LazyNutObject &LazyNutObject::operator=(LazyNutObject other)
//{
//    std::swap(other, *this);
//    return *this;
//}

//LazyNutObject::~LazyNutObject()
//{
//    delete domDoc;
//}


//void LazyNutObject::initProperties()
//{
//    XMLelement XMLroot = XMLelement(*domDoc);
//    //qDebug() << domDoc->toString();
//    if (!(XMLroot.label() == "this"))
//        qDebug () << "LazyNutObject: object description has no 'this' label";
//    _name = XMLroot.value();
//    _type = XMLroot["type"]();
//    if (_name.isEmpty())
//        qDebug () << "LazyNutObject: object description has empty name";
//    if (_type.isEmpty())
//        qDebug () << "LazyNutObject: object description has empty type";
//    // TODO: check type is admissible
//}


//QString LazyNutObject::getValue(const QString &label)
//{
//    return XMLelement(*domDoc)[label]();
//}

