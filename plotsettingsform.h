#ifndef PLOTSETTINGSFORM_H
#define PLOTSETTINGSFORM_H

#include <QWidget>
#include <QList>
#include <QDomDocument>
#include <QMap>

class QVBoxLayout;
class PlotSettingsBaseWidget;

class PlotSettingsForm : public QWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsForm(QDomDocument *domDoc, QWidget *parent = 0);
    ~PlotSettingsForm();
    QMap<QString,QString> getSettings();

signals:

public slots:
    void setFactorList(QStringList list);

private slots:
    void addWidget(QWidget *widget);

private:
    PlotSettingsBaseWidget *createWidget(QDomElement settingsElement);

    QList<PlotSettingsBaseWidget*> widgetList;
    QVBoxLayout *mainLayout;
    QDomDocument *domDoc;


};

#endif // PLOTSETTINGSFORM_H
