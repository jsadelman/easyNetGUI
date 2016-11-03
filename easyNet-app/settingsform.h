#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H


#include <QWidget>
#include <QList>
#include <QDomDocument>
#include <QMap>
#include <QSet>
#include <QTabWidget>
//#include "xmlelement.h"
#include "xmlaccessor.h"

class QVBoxLayout;
class PlotSettingsBaseWidget;


class SettingsForm: public QTabWidget
{
    Q_OBJECT
    Q_PROPERTY(bool useRFormat READ useRFormat WRITE setUseRFormat)
    Q_PROPERTY(QString name READ name WRITE setName)
public:
    explicit SettingsForm(QDomDocument *domDoc, QWidget *parent = 0);
    virtual ~SettingsForm();
    void build();
    QMap<QString, QString> getSettings();
    QStringList getSettingsCmdList(bool force = false);
    QString value(QString label);
    QStringList listLabels() {return XMLAccessor::listLabels(rootElement);}
    void setSetting(QString setting, QString value);
    void setCurrentTab(QString name);

    // setters and getters
    bool useRFormat() {return m_useRFormat;}
    QString name() {return m_name;}
    void setUseRFormat(bool useRFormat) {m_useRFormat = useRFormat;}
    void setName(QString name) {m_name = name;}
    QMap<QString, QString> defaultSettings() {return m_defaultSettings;}
    void setDefaultSettings(QMap<QString, QString> defaultSettings) {m_defaultSettings = defaultSettings;}
    bool allIsSet();

signals:
    void updateRequest();

protected slots:
    virtual void recordValueChange(QString oldValue, QString newValue);
    void checkDependencies();
    void updateDependees(QDomDocument *newDomDoc = nullptr);
    void updateSize();

protected:
    virtual void triggerUpdateDependees();
    void initDependersSet();
    PlotSettingsBaseWidget *createWidget(QDomElement &domElement);

    QString getSettingCmdLine(QString setting);
    virtual void substituteDependentValues(QDomElement& settingsElement);

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
    QString m_name;

    QMap<QString, QString> m_defaultSettings;
};

#endif // SETTINGSFORM_H
