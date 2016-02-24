#ifndef SETTINGSXML_H
#define SETTINGSXML_H

#include "xmlform.h"

class SettingsXML: public XMLForm
{
public:
    SettingsXML(const QDomElement& domElem, QWidget *parent = 0);
    ~SettingsXML();
    void setTopLabelValue(QString label, QString value) {m_label = label; m_value = value;}

protected:
    virtual void recursiveBuild() Q_DECL_OVERRIDE;
    virtual void buildNodeENelements() Q_DECL_OVERRIDE;
    virtual void buildChildrenENelements() Q_DECL_OVERRIDE;
    QString m_label;
    QString m_value;

};

#endif // SETTINGSXML_H
