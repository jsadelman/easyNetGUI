#ifndef PLOTSETTINGSBASEWIDGET_H
#define PLOTSETTINGSBASEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QDebug>
#include <QFrame>

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



class PlotSettingsBaseWidget : public QFrame//, PlotSettingsAbstractBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsBaseWidget(XMLelement settingsElement, QWidget *parent = 0);
    virtual ~PlotSettingsBaseWidget() {}
    QString name() {return settingsElement.label();}
    virtual QString value();
//    QList<QWidget*> extraWidgets() {return extraWidgetsList;}
    virtual void updateWidget(XMLelement xml);

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
    virtual QVariant getWidgetValue() {return QString();}
    virtual QString getValue();
    virtual void setValue(QString val);
    virtual void setWidgetValue(QVariant val){}
    bool isValueSet();
    bool hasDefault();
    virtual QVariant raw2widgetValue(QString val){return QVariant();}
    virtual QString widget2rawValue(QVariant val){return QString();}


    enum {RawEditMode, WidgetEditMode};
    int editMode;

    XMLelement settingsElement;
    bool valueSet;
    QString currentValue;

    QGridLayout *mainLayout;
    QLabel *nameLabel;
    QLineEdit *rawEdit;
    QStackedLayout *editStackedLayout;
    QWidget *editDisplayWidget;
    QLabel *commentLabel;
    QGroupBox *editModeButtonBox;
    QVBoxLayout *editModeButtonBoxLayout;
    QPushButton *debugButton;
    QRadioButton *rawEditButton;
    QRadioButton *widgetEditButton;
    QPushButton *defaultButton;

//    QList<QWidget*> extraWidgetsList;

};

class PlotSettingsNumericWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsNumericWidget(XMLelement settingsElement, QWidget *parent = 0);
    virtual ~PlotSettingsNumericWidget() {}

protected:
    void createEditWidget();
    virtual QVariant getWidgetValue() Q_DECL_OVERRIDE;
    virtual void setWidgetValue(QVariant val) Q_DECL_OVERRIDE;
    virtual QVariant raw2widgetValue(QString val) Q_DECL_OVERRIDE;
    virtual QString widget2rawValue(QVariant val) Q_DECL_OVERRIDE;



};


class PlotSettingsSingleChoiceWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsSingleChoiceWidget(XMLelement settingsElement, QWidget *parent = 0);
    virtual ~PlotSettingsSingleChoiceWidget() {}
    virtual void updateWidget(XMLelement xml) Q_DECL_OVERRIDE;

protected slots:
    void setupEditWidget();

protected:
    void createEditWidget();
    virtual QVariant getWidgetValue() Q_DECL_OVERRIDE;
    virtual void setWidgetValue(QVariant val) Q_DECL_OVERRIDE;
    virtual QVariant raw2widgetValue(QString val) Q_DECL_OVERRIDE;
    virtual QString widget2rawValue(QVariant val) Q_DECL_OVERRIDE;

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
    virtual void updateWidget(XMLelement xml) Q_DECL_OVERRIDE;


protected slots:
    void setupEditWidget();
    virtual void setRawMode(bool on);
    virtual void setWidgetMode(bool on);
    void updateEditDisplayWidget();
    virtual void resetDefault();


protected:
    void createEditWidget();
    virtual QVariant getWidgetValue() Q_DECL_OVERRIDE;
    virtual void setWidgetValue(QVariant val) Q_DECL_OVERRIDE;
    virtual QVariant raw2widgetValue(QString val) Q_DECL_OVERRIDE;
    virtual QString widget2rawValue(QVariant val) Q_DECL_OVERRIDE;

    void createListEdit();
    void updateValueFromEdit();

    LazyNutListWidget *factorList;
    LazyNutPairedListWidget *editExtraWidget;
};


#endif // PLOTSETTINGSBASEWIDGET_H
