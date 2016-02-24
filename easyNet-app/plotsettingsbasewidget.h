#ifndef PLOTSETTINGSBASEWIDGET_H
#define PLOTSETTINGSBASEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QDebug>
#include <QFrame>
#include <QDomElement>

#include "xmlelement.h"
#include "xmlaccessor.h"

class QDomDocument;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QStackedLayout;
class QStackedWidget;
class QLabel;
class QGroupBox;
class QPushButton;
class QRadioButton;
class QListWidget;
class QListWidgetItem;
class LazyNutListWidget;
class PairedListWidget;
class QAbstractItemModel;
class ObjectCacheFilter;

class PlotSettingsAbstractBaseWidget
{
public:
    virtual ~PlotSettingsAbstractBaseWidget() {}
    virtual QString name() = 0;
    virtual QString value() = 0;
    virtual QString settingMethod() = 0; // "setting" or "setting_object"

};



class PlotSettingsBaseWidget : public QFrame//, PlotSettingsAbstractBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsBaseWidget(QDomElement& settingsElement, bool _useRFormat = false, QWidget *parent = 0);
    virtual ~PlotSettingsBaseWidget() {}
    virtual QString name() {return XMLAccessor::label(settingsElement);}
    virtual QString value();
    virtual QString settingMethod();
    virtual void updateWidget(QDomElement& xml);
    void setValue(QString val);
    bool isDataframe();

public slots:
    void setValueSetTrue() {valueSet = true;}
    void emitValueChanged();

signals:
    void valueChanged(QString, QString); // old value, new value
    void levelsReady();


protected slots:
    virtual void resetDefault();
    virtual void setRawEditMode(bool on);
    virtual void setRawEditModeOn();
    virtual void setRawEditModeOff();

    void getLevels();

protected:

    void createDisplay();
    void createLevelsListModel();
    virtual void createEditWidget(){}
    virtual QVariant getWidgetValue() {return QString();}
    virtual QString getValue();
    virtual void setWidgetValue(QVariant val){}
    bool isValueSet();
    bool hasDefault();
    QString defaultValue();
    virtual QVariant raw2widgetValue(QString val){return QVariant();}
    virtual QString widget2rawValue(QVariant val){return QString();}


    enum {RawEditMode, WidgetEditMode};
    int editMode;

    QDomElement settingsElement;
//    XMLelement settingsElement;
    bool useRFormat;
    bool valueSet;
    QString currentValue;
    QAbstractItemModel *levelsListModel;
    ObjectCacheFilter *levelsCmdObjectWatcher;

    QVBoxLayout *vboxLayout;
    QGridLayout *gridLayout;
    QLabel *nameLabel;
    QLineEdit *rawEdit;
    QWidget *editDisplayWidget;
    QLabel *commentLabel;
    QGroupBox *editModeButtonBox;
    QVBoxLayout *editModeButtonBoxLayout;
    QPushButton *debugButton;
    QRadioButton *rawEditModeButton;
    QRadioButton *widgetEditButton;
    QPushButton *defaultButton;
};

class PlotSettingsNumericWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsNumericWidget(QDomElement& settingsElement, bool useRFormat, QWidget *parent = 0);
    virtual ~PlotSettingsNumericWidget() {}

protected:
    void createEditWidget() Q_DECL_OVERRIDE;
    virtual QVariant getWidgetValue() Q_DECL_OVERRIDE;
    virtual void setWidgetValue(QVariant val) Q_DECL_OVERRIDE;
    virtual QVariant raw2widgetValue(QString val) Q_DECL_OVERRIDE;
    virtual QString widget2rawValue(QVariant val) Q_DECL_OVERRIDE;



};


class PlotSettingsSingleChoiceWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsSingleChoiceWidget(QDomElement& settingsElement, bool useRFormat, QWidget *parent = 0);
    virtual ~PlotSettingsSingleChoiceWidget() {}
    virtual void updateWidget(QDomElement &xml) Q_DECL_OVERRIDE;

protected slots:
    void buildEditWidget();

protected:
    void createEditWidget();
    virtual QVariant getWidgetValue() Q_DECL_OVERRIDE;
    virtual void setWidgetValue(QVariant val) Q_DECL_OVERRIDE;
    virtual QVariant raw2widgetValue(QString val) Q_DECL_OVERRIDE;
    virtual QString widget2rawValue(QVariant val) Q_DECL_OVERRIDE;


};

class PlotSettingsMultipleChoiceWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsMultipleChoiceWidget(QDomElement& settingsElement, bool useRFormat, QWidget *parent = 0);
    virtual ~PlotSettingsMultipleChoiceWidget() {}
    virtual void updateWidget(QDomElement &xml) Q_DECL_OVERRIDE;


protected slots:
    virtual void setRawEditModeOn() Q_DECL_OVERRIDE;
    virtual void setRawEditModeOff() Q_DECL_OVERRIDE;
    void updateEditDisplayWidget();
    virtual void resetDefault();
    void buildEditWidget();

protected:
    void createEditWidget();

    virtual QVariant getWidgetValue() Q_DECL_OVERRIDE;
    virtual void setWidgetValue(QVariant val) Q_DECL_OVERRIDE;
    virtual QVariant raw2widgetValue(QString val) Q_DECL_OVERRIDE;
    virtual QString widget2rawValue(QVariant val) Q_DECL_OVERRIDE;



    LazyNutListWidget *factorList;
    QWidget *editExtraWidget;
};

class PlotSettingsFilenameWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsFilenameWidget(QDomElement& settingsElement, bool useRFormat, QWidget *parent = 0);
    virtual ~PlotSettingsFilenameWidget() {}

protected slots:
    void browseFilename();

protected:
    void createEditWidget() Q_DECL_OVERRIDE;
    virtual QVariant getWidgetValue() Q_DECL_OVERRIDE;
    virtual void setWidgetValue(QVariant val) Q_DECL_OVERRIDE;
    virtual QVariant raw2widgetValue(QString val) Q_DECL_OVERRIDE;
    virtual QString widget2rawValue(QVariant val) Q_DECL_OVERRIDE;
};



#endif // PLOTSETTINGSBASEWIDGET_H
