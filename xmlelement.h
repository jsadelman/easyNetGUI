#ifndef XMLELEMENT_H
#define XMLELEMENT_H

#include <QString>
#include <QStringList>

class QDomElement;


class XMLelement
{
public:
    XMLelement(QDomElement domElement);
//    XMLelement& operator=(XMLelement&&) {return *this;}
    bool isString() {return type == "string";}
    bool isInteger() {return type == "integer";}
    bool isObject() {return type == "object";}
    bool isCommand() {return type == "command";}
    bool isMap() {return type == "map";}
    bool isList() {return type == "list";}
    bool isENelements() {return type == "eNelements";}
    XMLelement firstChild(QString childType = QString());
    XMLelement nextSibling(QString siblingType = QString());
    bool isNull();
    QString label();
    QString value();
    QStringList list();

    QString operator ()();

    QString operator [](QString label);
private:
    QDomElement domElement;
    QString type;
};

#endif // XMLELEMENT_H
