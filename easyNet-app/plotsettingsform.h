#ifndef PLOTSETTINGSFORM_H
#define PLOTSETTINGSFORM_H

#include <QDomDocument>
#include <QMap>
#include <QSet>
#include <QTabWidget>
#include "xmlelement.h"
#include "xmlaccessor.h"
#include "settingsform.h"

class QVBoxLayout;
class PlotSettingsBaseWidget;

class PlotSettingsForm : public SettingsForm
{
    Q_OBJECT
    Q_PROPERTY(QString plotName READ plotName WRITE setPlotName)
public:
    explicit PlotSettingsForm(QDomDocument *domDoc, QWidget *parent = 0);
    QString plotName() {return m_plotName;}
    void setPlotName(QString plotName) {m_plotName = plotName;}


private slots:
    virtual void recordValueChange(QString oldValue, QString newValue);

protected:
    virtual void triggerUpdateDependees();
    virtual QString getSettingCmdLine(QString setting);
    virtual void substituteDependentValues(QDomElement& settingsElement);

    QString m_plotName;

};

#endif // PLOTSETTINGSFORM_H
