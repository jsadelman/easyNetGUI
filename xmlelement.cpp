#include <QDomDocument>
#include <QDebug>
#include "xmlelement.h"


XMLelement::XMLelement(QDomElement domElement)
    : domElement(domElement), type(domElement.tagName())
{
}

XMLelement XMLelement::firstChild(QString childType)
{
    return XMLelement(domElement.firstChildElement(childType));
}

XMLelement XMLelement::nextSibling(QString siblingType)
{
    return XMLelement(domElement.nextSiblingElement(siblingType));
}

bool XMLelement::isNull()
{
    return domElement.isNull();
}

QString XMLelement::label()
{
    return domElement.attribute("label");
}

QString XMLelement::value()
{
    return domElement.attribute("value");
}

QStringList XMLelement::list()
{
    // if isList()
    QStringList values;
    QDomElement element = domElement.firstChildElement();
    while (!element.isNull())
    {
        values.append((QString) XMLelement(element)());
        element = element.nextSiblingElement();
    }
    return values;
}

QString XMLelement::operator ()()
{
    if (isString() || isInteger() || isObject())
        return value();

    else if (isCommand())
         return list().join(" ");

    else
        return QString();
}


QString XMLelement::operator [](QString label)
{
    // if isMap()
    QDomElement element = domElement.firstChildElement();
    while (!element.isNull())
    {
        if (element.attribute("label") == label)
            return element.attribute("value");
        element = element.nextSiblingElement();
    }
    return QString();
}

