#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H


#include <QWidget>
#include <QList>
#include <QDomDocument>
#include <QMap>
#include <QSet>
#include <QTabWidget>
#include "xmlelement.h"

class QVBoxLayout;
class PlotSettingsBaseWidget;


class SettingsForm: public QTabWidget
{
    Q_OBJECT
    Q_PROPERTY(bool useRFormat READ useRFormat WRITE setUseRFormat)
public:
    explicit SettingsForm(QDomDocument *domDoc, QWidget *parent = 0);
    ~SettingsForm();
    void build();
    QStringList getSettingsCmdList();
    QString value(QString label);
    QStringList listLabels() {return XMLelement(rootElement).listLabels();}
    void setDefaultModelSetting(QString setting, QString value);

    // setters and getters
    bool useRFormat() {return m_useRFormat;}
    void setUseRFormat(bool useRFormat) {m_useRFormat = useRFormat;}
//    QMap<QString, QString> defaultSettings() {return m_defaultSettings;}
//    void setDefaultSettings(QMap<QString, QString> defaultSettings) {m_defaultSettings = defaultSettings;}


signals:
    void updateRequest();

private slots:
    void recordValueChange();
    virtual void checkDependencies();
    void updateDependees(QDomDocument *newDomDoc = nullptr);
    void updateSize();

private:
    void initDependersSet();
    PlotSettingsBaseWidget *createWidget(QDomElement domElement);
//    PlotSettingsBaseWidget *createWidget(XMLelement settingsElement);

    virtual QString getSettingCmdLine(QString setting);
    void substituteDependentValues(QDomElement domElement);

    QStringList tabOrder;
    QMap<QString, QVBoxLayout*> layoutMap;
    QMap<QString, QWidget*> twidgetMap;
    QMap<QString, PlotSettingsBaseWidget*> widgetMap;
    QMap<QString, bool> hasChanged;
    QVBoxLayout *mainLayout;
    QDomDocument *domDoc;
    QDomElement rootElement;
    QSet<QString> dependersSet;
    QString dependerOnUpdate;
    bool m_useRFormat;

//    QMap<QString, QString> m_defaultSettings;
};

#endif // SETTINGSFORM_H
