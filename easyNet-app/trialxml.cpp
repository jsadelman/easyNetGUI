#include "trialxml.h"
#include <QDebug>

TrialXML::TrialXML(const QDomElement &domElem, QWidget *parent)
    : ObjectXML(domElem, parent)
{
}

TrialXML::~TrialXML()
{
}

void TrialXML::recursiveBuild()
{
    QDomElement element = domElem.firstChildElement();
    while (!element.isNull())
    {
        // skip type and subtype
        if (domElem.attribute("label") == "this" &&
            (element.attribute("label") == "type" || element.attribute("label") == "subtype"))
        {
            element = element.nextSiblingElement();
            continue;
        }

        TrialXML *childForm = new TrialXML(element);
        childForm->build();
        // layout event parameters horizontally
        if (domElem.parentNode().toElement().attribute("label") == "events" &&
            domElem.parentNode().parentNode().toElement().attribute("label") == "this")
        {
            static_cast<QHBoxLayout*>(childrenLayout)->addWidget(childForm);
        }
        else
        {
            static_cast<QVBoxLayout*>(childrenLayout)->addWidget(childForm);
        }
        element = element.nextSiblingElement();
    }
    static_cast<QVBoxLayout*>(childrenLayout)->addStretch();
}

void TrialXML::buildFrameLabel()
{
    // layout event parameters horizontally
    if (domElem.parentNode().toElement().attribute("label") == "events" &&
        domElem.parentNode().parentNode().toElement().attribute("label") == "this")
    {
        setFrameStyle(QFrame::Box | QFrame::Sunken);
        setLineWidth(1);
        QVBoxLayout *frameLayout = new QVBoxLayout();
        setLayout(frameLayout);
        nodeLayout = new QHBoxLayout();
        frameLayout->addLayout(nodeLayout);
        buildLabel();

        if (domElem.hasChildNodes())
        {
            childrenLayout = new QHBoxLayout();
            frameLayout->addLayout(childrenLayout);
        }
    }
    else
        ObjectXML::buildFrameLabel();
}

void TrialXML::buildLabelValue()
{
    // if an event parameter use an external VBox layout and add a stretch at the end
    // so that event parameters are top-aligned in their horizontal layout
    if (domElem.parentNode().parentNode().toElement().attribute("label") == "events" &&
        domElem.parentNode().parentNode().parentNode().toElement().attribute("label") == "this")
    {
        QVBoxLayout *frameLayout = new QVBoxLayout();
        setLayout(frameLayout);
        ObjectXML::buildLabelValue();
        frameLayout->addLayout(nodeLayout);
        frameLayout->addStretch();
    }
    else
    {
        ObjectXML::buildLabelValue();
    }
}

