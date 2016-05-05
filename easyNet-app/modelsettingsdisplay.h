#ifndef MODELSETTINGSDISPLAY_H
#define MODELSETTINGSDISPLAY_H

#include <QScrollArea>
#include <QDomDocument>

class SettingsXML;
class ObjectCacheFilter;


class ModelSettingsDisplay: public QScrollArea
{
    Q_OBJECT
public:
    ModelSettingsDisplay(QWidget *parent = 0);
    void setCommand(QString command) {m_command = command;}

public slots:
    void buildForm(QString name);
    void buildForm(QDomDocument* domDoc);

private:
    SettingsXML *form;
    QString m_command;
    QString m_name;
     ObjectCacheFilter* modelFilter;

};

#endif // MODELSETTINGSDISPLAY_H
