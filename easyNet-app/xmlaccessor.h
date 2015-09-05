#ifndef XMLACCESSOR_H
#define XMLACCESSOR_H

#include <QDomElement>
#include <QStringList>


class XMLAccessor
{
public:
    static QString label(QDomElement& domElement) {return domElement.attribute("label");}
    static QString value(QDomElement& domElement) {return domElement.attribute("value");}
    static QString type(QDomElement& domElement) {return domElement.attribute("type");}
    static void setLabel(QDomElement& domElement, QString label) {domElement.setAttribute("label", label);}
    static void setValue(QDomElement& domElement, QString value) {domElement.setAttribute("value", value);}
    static QStringList listValues(QDomElement& domElement);
    static QStringList listLabels(QDomElement& domElement);
    static QString command(QDomElement& domElement);
    static QDomElement childElement(QDomElement& domElement, QString label);

};

#endif // XMLACCESSOR_H
