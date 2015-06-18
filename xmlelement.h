#ifndef XMLELEMENT_H
#define XMLELEMENT_H

#include <QString>
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>




class XMLelement
{
public:
    XMLelement(){}
    XMLelement(QDomDocument domDoc);
    XMLelement(QDomElement domElem);
    void setDomElement(QDomDocument domDoc);
    void setDomElement(QDomElement domElem);
    bool isString() {return tag == "string";}
    bool isInteger() {return tag == "integer";}
    bool isReal() {return tag == "real";}
    bool isObject() {return tag == "object";}
    bool isCommand() {return tag == "command";}
    bool isMap() {return tag == "map";}
    bool isList() {return tag == "list";}
    bool isENelements() {return tag == "eNelements";}
    bool isParameter() {return tag == "parameter";}
    XMLelement firstChild(QString childType = QString());
    XMLelement nextSibling(QString siblingType = QString());
    QString attribute(QString attr);
    bool hasAttribute(QString attr);
    bool isNull();
    QString label();
    QString value();
    QString type();
    void setLabel(QString label);
    void setValue(QString value);
    void setAttribute(QString name, QString value);
    QStringList listValues();
    QStringList listLabels();
    QString command();

    QString operator ()();

    XMLelement operator [](QString label);
private:
    QDomElement domElement;
    QString tag;
};

#endif // XMLELEMENT_H
