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

class PlotSettingsForm : public QTabWidget //QWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsForm(QDomDocument *domDoc, QString plotName,
                              QMap<QString,QString>, QWidget *parent = 0);
    ~PlotSettingsForm();
//    QMap<QString,QString> getSettings();
    QStringList getSettingsCmdList();
    QString value(QString label);
    QStringList listLabels() {return rootElement.listLabels();}

    void setDefaultModelSetting(QString setting, QString value);
signals:
    void updateRequest();
    void plotSettingFormCreated();

//public slots:
//    void setFactorList(QStringList list);

private slots:
//    void addWidget(QWidget *widget);
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
    QVBoxLayout *mainLayout;
    QDomDocument *domDoc;
    XMLelement rootElement;
    QSet<QString> dependersSet;
    QString dependerOnUpdate;
    QString plotName;
};

#endif // PLOTSETTINGSFORM_H
