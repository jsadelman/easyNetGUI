#ifndef TRIALXML_H
#define TRIALXML_H

#include "objectxml.h"

class TrialXML: public ObjectXML
{
public:
    TrialXML(const QDomElement& domElem, QWidget *parent = 0);
    ~TrialXML();

protected:
    virtual void recursiveBuild();
    virtual void buildFrameLabel();
    virtual void buildLabelValue();

};

#endif // TRIALXML_H
