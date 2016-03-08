#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H


#include <QWidget>
#include <QMap>
#include <QSharedPointer>


class QDomDocument;
class QLabel;
class QLineEdit;
class QPushButton;
class QScrollArea;
class PlotSettingsForm;
class QGridLayout;
class QToolButton;
class ObjectCacheFilter;
class ObjectUpdater;


class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    SettingsWidget(QString dataViewType, QWidget *parent = 0);
    ~SettingsWidget();
    void setSetting(QString setting, QString value);
    QMap<QString, QString> getSettings(QString name);

public slots:
    void sendSettings(QString name = QString());
    void newForm(QString name,
                 QString rScript,
                 QMap <QString,QString> defaultSettings,
                 int flags,
                 QList<QSharedPointer<QDomDocument> > infoList); // old newRPlot
    void setForm(QString name); // old setPlotSettings

signals:
    void dataViewCreated(QString, bool, bool, QList<QSharedPointer<QDomDocument> >); // old newRPlotCreated


protected slots:
    void buildSettingsForm(QString name, QDomDocument *domDoc,
                           QMap<QString, QString> defaultSettings=QMap<QString, QString>());
    void buildSettingsForm();
    void rebuildForm();
    void refreshForm();
    void setCurrentSettings(QDomDocument *settingsList) {currentSettings = settingsList;}
    void setCurrentPlotName(QString name) {currentName = name;}
    void removeForm(QString name);

protected:
    void createActions();
    void buildWidget();
    void clearForm();
    QString type(QString name);


    QString dataViewType; // dataframe_view or rplot
    QString currentName;
    QMap<QString, PlotSettingsForm*> formMap;
    QMap<QString, QString> typeMap;
    QDomDocument *currentSettings;
    ObjectCacheFilter *descriptionFilter;
    ObjectUpdater *descriptionUpdater;
    QAction *refreshAct;
    QAction *applyAct;
    QToolButton *refreshButton;
    QToolButton *applyButton;
    QLineEdit *nameEdit;
    QLineEdit *typeEdit;
    QScrollArea *formScrollArea;
};

#endif // SETTINGSWIDGET_H
