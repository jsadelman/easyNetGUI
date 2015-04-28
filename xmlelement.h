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
    bool isString() {return type == "string";}
    bool isInteger() {return type == "integer";}
    bool isReal() {return type == "real";}
    bool isObject() {return type == "object";}
    bool isCommand() {return type == "command";}
    bool isMap() {return type == "map";}
    bool isList() {return type == "list";}
    bool isENelements() {return type == "eNelements";}
    XMLelement firstChild(QString childType = QString());
    XMLelement nextSibling(QString siblingType = QString());
    QString attribute(QString attr);
    bool hasAttribute(QString attr);
    bool isNull();
    QString label();
    QString value();
    void setLabel(QString label);
    void setValue(QString value);
    void setAttribute(QString name, QString value);
    QStringList listValues();
    QStringList listLabels();

    QString operator ()();

    XMLelement operator [](QString label);
private:
    QDomElement domElement;
    QString type;
};

#endif // XMLELEMENT_H
