#ifndef PLOTSETTINGSFORM_H
#define PLOTSETTINGSFORM_H

#include <QWidget>
#include <QList>
#include <QDomDocument>
#include <QMap>
#include <QSet>
#include "xmlelement.h"

class QVBoxLayout;
class PlotSettingsBaseWidget;

class PlotSettingsForm : public QWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsForm(QDomDocument *domDoc, QString plotName, QWidget *parent = 0);
    ~PlotSettingsForm();
//    QMap<QString,QString> getSettings();
    QStringList getSettingsCmdList();

signals:
    void updateRequest();

//public slots:
//    void setFactorList(QStringList list);

private slots:
//    void addWidget(QWidget *widget);
    void checkDependencies();

private:
    void initDependersSet();
    PlotSettingsBaseWidget *createWidget(QDomElement settingsElement);
    PlotSettingsBaseWidget *createWidget(XMLelement settingsElement);
    QString getSettingCmdLine(QString setting);

    QList<PlotSettingsBaseWidget*> widgetList;
    QMap<QString, PlotSettingsBaseWidget*> widgetMap;
    QVBoxLayout *mainLayout;
    QDomDocument *domDoc;
    XMLelement rootElement;
    QSet<QString> dependersSet;
    QString dependerOnUpdate;
    QString plotName;
};

#endif // PLOTSETTINGSFORM_H
