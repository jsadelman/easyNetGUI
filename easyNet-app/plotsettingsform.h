#ifndef PLOTSETTINGSFORM_H
#define PLOTSETTINGSFORM_H

#include <QWidget>
#include <QList>
#include <QDomDocument>
#include <QMap>
#include <QSet>
#include <QTabWidget>
#include "xmlelement.h"

class QVBoxLayout;
class PlotSettingsBaseWidget;

class PlotSettingsForm : public QTabWidget
{
    Q_OBJECT
    Q_PROPERTY(QString plotName READ plotName WRITE setPlotName)
public:
    explicit PlotSettingsForm(QDomDocument *domDoc, QWidget *parent = 0);
    ~PlotSettingsForm();
    void build();
    QStringList getSettingsCmdList();
    QString value(QString label);
    QStringList listLabels() {return rootElement.listLabels();}
    void setDefaultModelSetting(QString setting, QString value);

    // setters and getters
    QString plotName() {return m_plotName;}
    void setPlotName(QString plotName) {m_plotName = plotName;}
    QMap<QString, QString> defaultSettings() {return m_defaultSettings;}
    void setDefaultSettings(QMap<QString, QString> defaultSettings) {m_defaultSettings = defaultSettings;}


signals:
    void updateRequest();

private slots:
    void recordValueChange();
    void checkDependencies();
    void updateDependees(QDomDocument *newDomDoc);
    void updateSize();

private:
    void initDependersSet();
    PlotSettingsBaseWidget *createWidget(QDomElement settingsElement);
    PlotSettingsBaseWidget *createWidget(XMLelement settingsElement);
    QString getSettingCmdLine(QString setting);

    QStringList tabOrder;
    QMap<QString, QVBoxLayout*> layoutMap;
    QMap<QString, QWidget*> twidgetMap;
    QMap<QString, PlotSettingsBaseWidget*> widgetMap;
    QMap<QString, bool> hasChanged;
    QVBoxLayout *mainLayout;
    QDomDocument *domDoc;
    XMLelement rootElement;
    QSet<QString> dependersSet;
    QString dependerOnUpdate;
    bool useRFormat;

    QString m_plotName;
    QMap<QString, QString> m_defaultSettings;
};

#endif // PLOTSETTINGSFORM_H
