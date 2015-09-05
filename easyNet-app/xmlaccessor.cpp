#include "xmlaccessor.h"


QStringList XMLAccessor::listValues(QDomElement &domElement)
{
    QStringList values;
    QDomElement element = domElement.firstChildElement();
    while (!element.isNull())
    {
        values.append(XMLAccessor::value(element));
        element = element.nextSiblingElement();
    }
    return values;
}

QStringList XMLAccessor::listLabels(QDomElement &domElement)
{
    QStringList labels;
    QDomElement element = domElement.firstChildElement();
    while (!element.isNull())
    {
        labels.append(XMLAccessor::label(element));
        element = element.nextSiblingElement();
    }
    return labels;
}

QString XMLAccessor::command(QDomElement &domElement)
{
    if (domElement.tagName() == "command")
        return (XMLAccessor::listValues(domElement)).join(' ');

    else
        return QString();
}

QDomElement XMLAccessor::childElement(QDomElement &domElement, QString label)
{
    QDomElement element = domElement.firstChildElement();
    while (!element.isNull())
    {
        if (element.attribute("label") == label)
            return element;
        element = element.nextSiblingElement();
    }
    return QDomElement();
}
