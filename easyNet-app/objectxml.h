#ifndef OBJECTXML_H
#define OBJECTXML_H

#include "xmlform.h"

class ObjectXML: public XMLForm
{
public:
    ObjectXML(const QDomElement& domElem, QWidget *parent = 0);
    ~ObjectXML();
    virtual void buildNodeObject();

};

#endif // OBJECTXML_H
