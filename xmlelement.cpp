#include <QDomDocument>
#include <QDebug>
#include "xmlelement.h"


XMLelement::XMLelement(QDomDocument domDoc)
{
    domElement = domDoc.documentElement();
    type = domElement.tagName();
    if (!isENelements())
        qDebug() << "error: root xml tag is not <eNelement>";

}

XMLelement::XMLelement(QDomElement domElem)
    : domElement(domElem), type(domElem.tagName())
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

QString XMLelement::attribute(QString attr)
{
    return domElement.attribute(attr);
}

bool XMLelement::hasAttribute(QString attr)
{
    return domElement.hasAttribute(attr);
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

QStringList XMLelement::listValues()
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

QStringList XMLelement::listLabels()
{
    QStringList labels;
    QDomElement element = domElement.firstChildElement();
    while (!element.isNull())
    {
        labels.append(XMLelement(element).label());
        element = element.nextSiblingElement();
    }
    return labels;
}

QString XMLelement::operator ()()
{
    if (isString() || isInteger() || isReal() || isObject())
        return value();

    else if (isCommand())
         return listValues().join(" ");

    else
        return QString();
}


XMLelement XMLelement::operator [](QString label)
{
    // if isMap()
    QDomElement element = domElement.firstChildElement();
    while (!element.isNull())
    {
        if (element.attribute("label") == label)
            return XMLelement(element);
        element = element.nextSiblingElement();
    }
    return XMLelement(QDomElement());
}

