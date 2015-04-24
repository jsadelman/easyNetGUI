#ifndef PLOTSETTINGSBASEWIDGET_H
#define PLOTSETTINGSBASEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QDebug>

#include "xmlelement.h"

class QDomDocument;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QStackedLayout;
class QLabel;
class QGroupBox;
class QPushButton;
class QRadioButton;
class QListWidget;
class QListWidgetItem;
class LazyNutListWidget;
class LazyNutPairedListWidget;

class PlotSettingsAbstractBaseWidget
{
public:
    virtual ~PlotSettingsAbstractBaseWidget() {}
    virtual QString name() = 0;
    virtual QString value() = 0;
    virtual QList<QWidget*> extraWidgets() = 0;

};



class PlotSettingsBaseWidget : public QWidget//, PlotSettingsAbstractBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsBaseWidget(XMLelement settingsElement, QWidget *parent = 0);
    virtual ~PlotSettingsBaseWidget() {}
    QString name() {return settingsElement.label();}
    virtual QString value();
    QList<QWidget*> extraWidgets() {return extraWidgetsList;}

signals:
    void valueChanged();
    void resetRequest();

protected slots:
    virtual void resetDefault();
    virtual void setRawMode(bool on);
    virtual void setWidgetMode(bool on);
    void emitValueChanged();
    void setValueSetTrue() {valueSet = true;}

protected:

    virtual void createDisplay();
    virtual void createEditWidget(){}
    virtual QString getWidgetValue() {return QString();}
    virtual QString getValue();
    virtual void setValue(QString val);
    virtual void setWidgetValue(QString val){}
    bool isValueSet();
    bool hasDefault();
    virtual QString raw2widgetValue(QString val){}
    virtual QString widget2rawValue(QString val){}


    enum {RawEditMode, WidgetEditMode};
    int editMode;

    XMLelement settingsElement;
    bool valueSet;
    QString currentValue;

    QGridLayout *mainLayout;
    QLabel *nameLabel;
    QLineEdit *rawEdit;
    QStackedLayout *editStackedLayout;
    QWidget *editWidget;
    QLabel *commentLabel;
    QGroupBox *editModeButtonBox;
    QVBoxLayout *editModeButtonBoxLayout;
    QPushButton *debugButton;
    QRadioButton *rawEditButton;
    QRadioButton *widgetEditButton;
    QPushButton *defaultButton;

    QList<QWidget*> extraWidgetsList;

};

class PlotSettingsNumericWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsNumericWidget(XMLelement settingsElement, QWidget *parent = 0);
    virtual ~PlotSettingsNumericWidget() {}

protected:
    void createEditWidget();
    virtual QString getWidgetValue();
    virtual void setWidgetValue(QString val);
    virtual QString raw2widgetValue(QString val);
    virtual QString widget2rawValue(QString val);



};


class PlotSettingsSingleChoiceWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsSingleChoiceWidget(XMLelement settingsElement, QWidget *parent = 0);
    virtual ~PlotSettingsSingleChoiceWidget() {}


protected slots:
    void currentTextChangedFilter(QString value);
    void setupEditWidget();

protected:
    void createEditWidget();
    virtual QString getWidgetValue();
    virtual void setWidgetValue(QString val);
    virtual QString raw2widgetValue(QString val);
    virtual QString widget2rawValue(QString val);

    void createListEdit();
    void updateValueFromEdit();

    LazyNutListWidget *factorList;
};

class PlotSettingsMultipleChoiceWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsMultipleChoiceWidget(XMLelement settingsElement, QWidget *parent = 0);
    virtual ~PlotSettingsMultipleChoiceWidget() {}
    virtual QString value();


protected slots:
    void setupEditWidget();
    virtual void setRawMode(bool on);
    virtual void setWidgetMode(bool on);
    void updateEditWidget();
    virtual void resetDefault();


protected:
    void createEditWidget();
    virtual QString getWidgetValue();
    virtual void setWidgetValue(QString val);
    virtual QString raw2widgetValue(QString val);
    virtual QString widget2rawValue(QString val);

    void createListEdit();
    void updateValueFromEdit();

    LazyNutListWidget *factorList;
    LazyNutPairedListWidget *editExtraWidget;
};



/*

class PlotSettingsFactorWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
//    explicit PlotSettingsFactorWidget(QString name, QString value, QString comment,
//                                    QString defaultValue, QWidget *parent = 0);
    explicit PlotSettingsFactorWidget(XMLelement settingsElement, QWidget *parent = 0);

    virtual QString value();



protected:
    virtual void createValueEdit();

private slots:
    void addItems();
    void removeItems();
    void setFactorList(QStringList list);

    void updateListEdit();
    void debugValueChanged()
    {
        qDebug () << "valueChanged() emitted by " << name();
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
*/

#endif // PLOTSETTINGSBASEWIDGET_H
