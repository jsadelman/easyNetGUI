#ifndef PLOTSETTINGSBASEWIDGET_H
#define PLOTSETTINGSBASEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QDebug>

#include "xmlelement.h"

class QDomDocument;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QPushButton;
class QListWidget;
class QListWidgetItem;
class LazyNutListWidget;

class ValueEdit: public QLineEdit
{
    Q_OBJECT
public:
    explicit ValueEdit(QWidget *parent = 0);

signals:
    void doubleClicked();



protected:
    virtual void mouseDoubleClickEvent(QMouseEvent * mouseEvent) Q_DECL_OVERRIDE;
};

class PlotSettingsBaseWidget : public QWidget
{
    Q_OBJECT
public:
//    explicit PlotSettingsBaseWidget(QString name, QString value, QString comment,
//                                    QString defaultValue, QWidget *parent = 0);
    explicit PlotSettingsBaseWidget(XMLelement settingsElement, QWidget *parent = 0);
    virtual ~PlotSettingsBaseWidget() {}
    QString getName() {return settingsElement.label();}   // {return name;}
    virtual QString getValue();
    QList<QWidget*> extraWidgets() {return extraWidgetsList;}

signals:
    void valueChanged();
    void addWidget(QWidget*);

public slots:

protected slots:
    void displayComment();

protected:
    virtual void createDisplay();
    virtual void createValueEdit();

    XMLelement settingsElement;

    QString name;
    QString value;
    QString comment;
    QString defaultValue;
    QPushButton *nameButton;
    QHBoxLayout *displayLayout;
    QVBoxLayout *mainLayout;
//    QPushButton *commentButton;
    QWidget *valueEdit;
    QList<QWidget*> extraWidgetsList;

};

class PlotSettingsNumericWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
//    explicit PlotSettingsNumericWidget(QString name, QString value, QString comment,
//                                    QString defaultValue, QWidget *parent = 0);
    explicit PlotSettingsNumericWidget(XMLelement settingsElement, QWidget *parent = 0);

    virtual QString getValue();

protected:
    virtual void createValueEdit();
};

class PlotSettingsDataframeWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsDataframeWidget(XMLelement settingsElement, QWidget *parent = 0);

    virtual QString getValue();

protected slots:
    void currentTextChangedFilter(QString value);


protected:
    virtual void createValueEdit();
    void createListEdit();
    void updateValueFromEdit();

    LazyNutListWidget *factorList;
    QPushButton *editButton;
};


class PlotSettingsFactorWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
//    explicit PlotSettingsFactorWidget(QString name, QString value, QString comment,
//                                    QString defaultValue, QWidget *parent = 0);
    explicit PlotSettingsFactorWidget(XMLelement settingsElement, QWidget *parent = 0);

    virtual QString getValue();



protected:
    virtual void createValueEdit();

private slots:
    void addItems();
    void removeItems();
    void setFactorList(QStringList list);

    void updateListEdit();
    void debugValueChanged()
    {
        qDebug () << "valueChanged() emitted by " << getName();
    }

private:
    void createListEdit();
    void getLevels();
    QString formatFactorStringForR(QString factorString);
    QString formatFactorStringForDisplay(QString factorString);
    QStringList factorString2list(QString factorString);
    void moveItems(QListWidget *fromList, QListWidget *toList, QList<QListWidgetItem *> items);
    void updateValueFromEdit();

    QListWidget *factorList;
    QListWidget *selectedList;
    QPushButton *editButton;
};


#endif // PLOTSETTINGSBASEWIDGET_H
