#include "settingsxml.h"

#include <QDebug>

SettingsXML::SettingsXML(const QDomElement &domElem, QWidget *parent)
   : XMLForm(domElem, parent), m_label(""), m_value("")
{
}

SettingsXML::~SettingsXML()
{
}

void SettingsXML::recursiveBuild()
{
    QDomElement element = domElem.firstChildElement();
    while (!element.isNull())
    {
        if (domElem.parentNode().toElement().tagName() == "eNelements" &&
            !(element.attribute("label") == "value" || element.attribute("label") == "comment"))
        {
            // skip type, grouping, default, etc.
            element = element.nextSiblingElement();
            continue;
        }
        SettingsXML *childForm = new SettingsXML(element);
        childForm->build();
        static_cast<QVBoxLayout*>(childrenLayout)->addWidget(childForm);
        element = element.nextSiblingElement();
    }
    static_cast<QVBoxLayout*>(childrenLayout)->addStretch();
}

void SettingsXML::buildNodeENelements()
{
    domElem.setAttribute("label", m_label);
    domElem.setAttribute("value", m_value);
    buildLabelValue();
}

void SettingsXML::buildChildrenENelements()
{
    indentChildren();
}

