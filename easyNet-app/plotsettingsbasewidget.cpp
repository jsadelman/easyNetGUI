#include <QtWidgets>

#include "plotsettingsbasewidget.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"
#include "pairedlistwidget.h"
#include "bracketedparser.h"
#include "simplelistmodel.h"
#include "selectfromlistmodel.h"
#include "objectcataloguefilter.h"
#include "proxymodelextrarows.h"
#include "xmlaccessor.h"
#include <QMetaObject>


PlotSettingsBaseWidget::PlotSettingsBaseWidget(QDomElement& settingsElement, bool useRFormat, QWidget *parent)
    : settingsElement(settingsElement), useRFormat(useRFormat), levelsListModel(nullptr), levelsCmdObjectWatcher(nullptr), editDisplayWidget(nullptr), QFrame(parent)
{
    createDisplay();
    createLevelsListModel();
    editMode = RawEditMode;
}


void PlotSettingsBaseWidget::createDisplay()
{
    gridLayout = new QGridLayout;
    nameLabel = new QLabel;
//    QString labelText = settingsElement["pretty name"]().isEmpty() ? name() : settingsElement["pretty name"]();
    QString labelText = XMLAccessor::label(settingsElement);
    if (!hasDefault())
        labelText.append("*");

    nameLabel->setText(labelText);
    nameLabel->setStyleSheet("QLabel {"
                             "font-weight: bold;"
                             "}");
    rawEdit = new QLineEdit;
    if (!hasDefault())
        rawEdit->setPlaceholderText("*mandatory field*");

    QDomElement settingsValue = XMLAccessor::childElement(settingsElement, "value");
    QDomElement settingsDefault = XMLAccessor::childElement(settingsElement, "default");
    QDomElement settingsComment = XMLAccessor::childElement(settingsElement, "comment");

    if (XMLAccessor::value(settingsValue).isEmpty())
    {
        if (hasDefault())
            rawEdit->setText(XMLAccessor::value(settingsDefault));
    }
     else
        rawEdit->setText(XMLAccessor::value(settingsValue));


    connect(rawEdit, SIGNAL(textChanged(QString)), this, SLOT(emitValueChanged()));
    commentLabel = new QLabel;
    commentLabel->setText(XMLAccessor::value(settingsComment));
    commentLabel->setWordWrap(true);
    commentLabel->setStyleSheet("QLabel {"
                                "background-color: lightyellow;"
                                "border: 1px solid black;"
                                "padding: 4px;"
                                "font-style: italic;"
                                "}");
    rawEditModeButton = new QRadioButton("Manual input");
    rawEditModeButton->setEnabled(false);
    rawEditModeButton->setChecked(true);
    connect(rawEditModeButton, SIGNAL(clicked(bool)), this, SLOT(setRawEditMode(bool)));


    if (hasDefault())
    {
        defaultButton = new QPushButton("Default");
        defaultButton->setToolTip(QString("Reset default value: %1").arg(XMLAccessor::value(settingsDefault)));
    }
    else
    {
        defaultButton = new QPushButton("Unset");
        defaultButton->setToolTip("Leave the value unspecified");
    }
    connect(defaultButton, SIGNAL(clicked()), this, SLOT(resetDefault()));
    connect(defaultButton, SIGNAL(clicked()), this, SLOT(emitValueChanged()));

    gridLayout->addWidget(nameLabel, 0, 0);

    gridLayout->addWidget(defaultButton, 0, 2);
    gridLayout->addWidget(commentLabel, 1, 0, 1, 2);
    gridLayout->addWidget(rawEditModeButton, 1, 2);
    vboxLayout = new QVBoxLayout;
    vboxLayout->addLayout(gridLayout);
    vboxLayout->addWidget(rawEdit);
    rawEdit->hide();
//    vboxLayout->addStretch();
    setLayout(vboxLayout);

    valueSet =  hasDefault() ?
                XMLAccessor::value(settingsValue) != XMLAccessor::value(settingsDefault) :
                !(XMLAccessor::value(settingsValue)).isEmpty();

    currentValue = XMLAccessor::value(settingsValue);


    // set max size to fit text
    // http://stackoverflow.com/questions/6639012/minimum-size-width-of-a-qpushbutton-that-is-created-from-code
//    QSize textSize = nameButton->fontMetrics().size(Qt::TextShowMnemonic, nameButton->text());
//    QStyleOptionButton opt;
//    opt.initFrom(nameButton);
//    opt.rect.setSize(textSize);
//    nameButton->setMaximumSize(nameButton->style()->sizeFromContents(QStyle::CT_PushButton,
//                                        &opt, textSize, nameButton));


    //    setAttribute(Qt::WA_AlwaysShowToolTips);
}

void PlotSettingsBaseWidget::createLevelsListModel()
{
    delete levelsCmdObjectWatcher;
    levelsCmdObjectWatcher = nullptr;
    delete levelsListModel;
    levelsListModel = nullptr;

    QDomElement levelsElement = XMLAccessor::childElement(settingsElement, "levels");
    if (levelsElement.isNull())
        return;
    else if (levelsElement.tagName() == "parameter")
    {
        levelsListModel = new ObjectCatalogueFilter(this);
        static_cast<ObjectCatalogueFilter*>(levelsListModel)->setType(XMLAccessor::type(levelsElement));

    }
    else if (levelsElement.tagName() == "command")
    {
        levelsListModel = new StringListModel(QStringList(), this);
        levelsCmdObjectWatcher = new ObjectCatalogueFilter(this);
        QStringList objectsInCmd;
        QDomElement cmdElement = levelsElement.firstChildElement("object");
        while (!cmdElement.isNull())
        {
            objectsInCmd.append(XMLAccessor::value(cmdElement));
            cmdElement = cmdElement.nextSiblingElement("object");
        }
        levelsCmdObjectWatcher->setNameList(objectsInCmd);
        connect(levelsCmdObjectWatcher, SIGNAL(objectModified(QString)),
                this, SLOT(getLevels()));
        connect(levelsCmdObjectWatcher, &ObjectCatalogueFilter::objectDestroyed, [=]()
        {
            static_cast<StringListModel*>(levelsListModel)->updateList(QStringList());
        });
    }
    else
        return;
}

QString PlotSettingsBaseWidget::getValue()
{
    switch(editMode)
    {
    case RawEditMode:
        return rawEdit->text();
    case WidgetEditMode:
        return widget2rawValue(getWidgetValue());
    default:
        return QString();
    }
}

void PlotSettingsBaseWidget::setValue(QString val)
{
    //settingsElement["value"].setValue(val);
    QDomElement valueElement = XMLAccessor::childElement(settingsElement, "value");
    qDebug() << "setValue element is null" <<  valueElement.isNull();
    XMLAccessor::setValue(valueElement, val);


    qDebug() << "setValue:" << val << "check" << XMLAccessor::value(valueElement);

    switch(editMode)
    {
    case RawEditMode:
    {
        rawEdit->setText(val);
        break;
    }
    case WidgetEditMode:
        setWidgetValue(raw2widgetValue(val));
    }
}

bool PlotSettingsBaseWidget::isValueSet()
{
    return valueSet || rawEdit->isModified();
}

bool PlotSettingsBaseWidget::hasDefault()
{
    return (XMLAccessor::listLabels(settingsElement)).contains("default");
}


QString PlotSettingsBaseWidget::value()
{
    if (isValueSet())
        return getValue();

    return QString();
}

QString PlotSettingsBaseWidget::settingMethod()
{
    QDomElement typeElement = XMLAccessor::childElement(settingsElement, "type");
    return XMLAccessor::value(typeElement) == "dataframe" ? "setting_object" : "setting";
}

void PlotSettingsBaseWidget::updateWidget(QDomElement &xml)
{
    settingsElement = xml;
    QDomElement levelsElement = XMLAccessor::childElement(settingsElement, "levels");
    if (!levelsListModel || levelsElement.isNull())
        createLevelsListModel();
}


void PlotSettingsBaseWidget::resetDefault()
{
    QDomElement defaultElement = XMLAccessor::childElement(settingsElement, "default");
    QString defaultValue = hasDefault() ? XMLAccessor::value(defaultElement) : QString();
    if (valueSet)
    {
        setValue(defaultValue);
        valueSet = false;
        if (currentValue != getValue())
        {
            currentValue = getValue();
            emit valueChanged();
        }
    }
}



void PlotSettingsBaseWidget::setRawEditMode(bool on)
{
    if (on)
    {
        int ret = QMessageBox::warning(this, tr("Leaving quick edit mode.\n"),
                                    tr("Switching to raw edit mode does preserve the current value for this setting, "
                                       "but it does not allow to switch back to quick edit mode without a value reset.\n"
                                       "Do you want to proceed?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
        if (ret == QMessageBox::Ok)
        {
            setRawEditModeOn();
        }

        else
            rawEditModeButton->setChecked(false);
    }
    else
    {
        int ret = QMessageBox::warning(this, tr("Leaving Manual edit mode.\n"),
                  tr("Switching to quick edit mode resets this setting to its default value or unsets it if no default is defined.\n"
                     "Do you want to proceed?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
        if (ret == QMessageBox::Ok)
        {
            setRawEditModeOff();
            resetDefault();
        }
        else
            rawEditModeButton->setChecked(true);
    }
}

void PlotSettingsBaseWidget::setRawEditModeOn()
{
    rawEdit->setText(widget2rawValue(getWidgetValue()));
    rawEdit->show();
    if (editDisplayWidget)
        editDisplayWidget->hide();
    editMode = RawEditMode;
}

void PlotSettingsBaseWidget::setRawEditModeOff()
{
    rawEdit->hide();
    if (editDisplayWidget)
        editDisplayWidget->show();
    editMode = WidgetEditMode;
}


void PlotSettingsBaseWidget::emitValueChanged()
{
    if (currentValue != getValue())
    {
        currentValue = getValue();
        valueSet = true;
        // write on XML
        QDomElement valueElement = XMLAccessor::childElement(settingsElement, "value");
        XMLAccessor::setValue(valueElement, currentValue);

        emit valueChanged();
        qDebug() << "In PlotSettingsBaseWidget, emitValueChanged" << currentValue;

    }
}

void PlotSettingsBaseWidget::getLevels()
{
    // check that all object fields in the levels command have been filled
    QDomElement levelsElement = XMLAccessor::childElement(settingsElement, "levels");
    QDomElement cmdToken = levelsElement.firstChildElement("object");
    while (!cmdToken.isNull())
    {
        if ((XMLAccessor::value(cmdToken)).isEmpty())
            return;

        cmdToken = cmdToken.nextSiblingElement("object");
    }
    // get the levels
    LazyNutJobParam *param = new LazyNutJobParam;
    param->cmdList = QStringList({(XMLAccessor::command(levelsElement)).prepend("xml ")});
    param->logMode &= ECHO_INTERPRETER; // debug purpose
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(qobject_cast<StringListModel*>(levelsListModel), SLOT(updateList(QStringList)));
    param->setEndOfJobReceiver(this, SIGNAL(levelsReady()));
    SessionManager::instance()->setupJob(param, sender());
}


//////////// PlotSettingsNumericWidget



PlotSettingsNumericWidget::PlotSettingsNumericWidget(QDomElement &domElement, bool useRFormat, QWidget *parent)
    : PlotSettingsBaseWidget(domElement, useRFormat, parent)
{
    createEditWidget();
}

void PlotSettingsNumericWidget::setWidgetValue(QVariant val)
{
    QDomElement valueElement = XMLAccessor::childElement(settingsElement, "value");
    if (valueElement.tagName() == "real")
        static_cast<QDoubleSpinBox*>(editDisplayWidget)->setValue(val.toDouble());
    else
        static_cast<QSpinBox*>(editDisplayWidget)->setValue(val.toInt());
}


QVariant PlotSettingsNumericWidget::getWidgetValue()
{
    QDomElement valueElement = XMLAccessor::childElement(settingsElement, "value");
    if (valueElement.tagName() == "real")
        return QString::number(static_cast<QDoubleSpinBox*>(editDisplayWidget)->value());
    else
        return QString::number(static_cast<QSpinBox*>(editDisplayWidget)->value());
}

QVariant PlotSettingsNumericWidget::raw2widgetValue(QString val)
{
    return val;
}

QString PlotSettingsNumericWidget::widget2rawValue(QVariant val)
{
    return val.toString();
}

void PlotSettingsNumericWidget::createEditWidget()
{
    QDomElement valueElement = XMLAccessor::childElement(settingsElement, "value");
    if (valueElement.tagName() == "real")
    {
        editDisplayWidget = new QDoubleSpinBox;
        setWidgetValue(XMLAccessor::value(valueElement));
        connect(static_cast<QDoubleSpinBox*>(editDisplayWidget), SIGNAL(valueChanged(double)),
                this, SLOT(emitValueChanged()));
    }
    else
    {
        editDisplayWidget = new QSpinBox;
        setWidgetValue(XMLAccessor::value(valueElement));
        connect(static_cast<QSpinBox*>(editDisplayWidget), SIGNAL(valueChanged(int)),
                this, SLOT(emitValueChanged()));
    }
    currentValue = XMLAccessor::value(valueElement);
    valueSet = !currentValue.isEmpty();
    gridLayout->addWidget(editDisplayWidget, 0, 1);
    rawEditModeButton->setEnabled(true);
    rawEditModeButton->setChecked(false);
    setRawEditModeOff();
}



//////////// PlotSettingsSingleChoiceWidget



PlotSettingsSingleChoiceWidget::PlotSettingsSingleChoiceWidget(QDomElement &domElement, bool useRFormat, QWidget *parent)
    : PlotSettingsBaseWidget(domElement, useRFormat, parent)
{
    connect(this, SIGNAL(levelsReady()), this, SLOT(buildEditWidget()));
    createEditWidget();
}

void PlotSettingsSingleChoiceWidget::updateWidget(QDomElement& xml)
{
    PlotSettingsBaseWidget::updateWidget(xml);

    createEditWidget();
}


void PlotSettingsSingleChoiceWidget::createEditWidget()
{
    QDomElement levelsElement = XMLAccessor::childElement(settingsElement, "levels");

    if (levelsElement.tagName() == "parameter")
        buildEditWidget();
    else if (levelsElement.tagName() == "command")
        getLevels();
    else
    {
        rawEditModeButton->setEnabled(false);
        rawEditModeButton->setChecked(true);
        setRawEditModeOn();
    }
}



void PlotSettingsSingleChoiceWidget::buildEditWidget()
{
    delete editDisplayWidget;
    editDisplayWidget = nullptr;

    editDisplayWidget = new QComboBox;
    static_cast<QComboBox*>(editDisplayWidget)->setModel(levelsListModel);

    QDomElement valueElement = XMLAccessor::childElement(settingsElement, "value");
    currentValue = XMLAccessor::value(valueElement);
    setWidgetValue(raw2widgetValue(currentValue));
    valueSet = !currentValue.isEmpty();
    connect(static_cast<QComboBox*>(editDisplayWidget),SIGNAL(currentIndexChanged(int)),
            this, SLOT(emitValueChanged()));
//    connect(static_cast<QComboBox*>(editDisplayWidget),SIGNAL(currentTextChanged(QString)),
//            this, SLOT(emitValueChanged()));
    connect(static_cast<QComboBox*>(editDisplayWidget),  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=](int index){
        qDebug() << "PlotSettingsSingleChoiceWidget index changed" << index;
    });
    gridLayout->addWidget(editDisplayWidget, 0, 1);
    rawEditModeButton->setEnabled(true);
    rawEditModeButton->setChecked(false);
    setRawEditModeOff();
    valueSet = !currentValue.isEmpty();
    currentValue = value();
}

QVariant PlotSettingsSingleChoiceWidget::getWidgetValue()
{
    if (editDisplayWidget)
        return static_cast<QComboBox*>(editDisplayWidget)->currentText();

    return QString();
}

void PlotSettingsSingleChoiceWidget::setWidgetValue(QVariant val)
{
    static_cast<QComboBox*>(editDisplayWidget)->setCurrentIndex(
                static_cast<QComboBox*>(editDisplayWidget)->findText(val.toString()));
}

QVariant PlotSettingsSingleChoiceWidget::raw2widgetValue(QString val)
{
    if (val.isEmpty() || val == "NULL")
        return QString();
    QDomElement typeElement = XMLAccessor::childElement(settingsElement, "type");
    return ((XMLAccessor::value(typeElement)) == "factor" && useRFormat) ?
        val.remove(QRegExp("^\\s*c\\(|\\)\\s*$|\"")).simplified() : val;
}

QString PlotSettingsSingleChoiceWidget::widget2rawValue(QVariant val)
{
    QDomElement defaultElement = XMLAccessor::childElement(settingsElement, "default");
    QDomElement typeElement = XMLAccessor::childElement(settingsElement, "type");
    QString stringVal = val.toString();
    if (stringVal.isEmpty())
    {
        if (!hasDefault())
            return QString();
        else if ((XMLAccessor::value(defaultElement)) == "NULL")
            return "NULL";
    }
    return ((XMLAccessor::value(typeElement)) == "factor" && useRFormat) ?
            QString("c(\"%1\")").arg(stringVal) : stringVal;
}

//////////////////////// PlotSettingsMultipleChoiceWidget


PlotSettingsMultipleChoiceWidget::PlotSettingsMultipleChoiceWidget(QDomElement& domElement, bool useRFormat, QWidget *parent)
    : editExtraWidget(nullptr), PlotSettingsBaseWidget(domElement, useRFormat, parent)
{
    connect(this, SIGNAL(levelsReady()), this, SLOT(buildEditWidget()));
    createEditWidget();
}

void PlotSettingsMultipleChoiceWidget::updateWidget(QDomElement& xml)
{
    PlotSettingsBaseWidget::updateWidget(xml);

    createEditWidget();
}


void PlotSettingsMultipleChoiceWidget::createEditWidget()
{
    QDomElement levelsElement = XMLAccessor::childElement(settingsElement, "levels");

    if (levelsElement.tagName() == "parameter")
        buildEditWidget();
    else if (levelsElement.tagName() == "command")
        getLevels();
    else
    {
        rawEditModeButton->setEnabled(false);
        rawEditModeButton->setChecked(true);
        setRawEditModeOn();
    }
}

void PlotSettingsMultipleChoiceWidget::buildEditWidget()
{
    delete editDisplayWidget;
    editDisplayWidget = nullptr;
    delete editExtraWidget;
    editExtraWidget = nullptr;

    editDisplayWidget = new QLineEdit();
    static_cast<QLineEdit*>(editDisplayWidget)->setReadOnly(true);
    gridLayout->addWidget(editDisplayWidget, 0,1);
    editExtraWidget = new PairedListWidget(levelsListModel);

    vboxLayout->addWidget(editExtraWidget);

    QDomElement valueElement = XMLAccessor::childElement(settingsElement, "value");
    currentValue = XMLAccessor::value(valueElement);
    setWidgetValue(raw2widgetValue(currentValue));
    updateEditDisplayWidget();

    valueSet = !currentValue.isEmpty();

    connect(static_cast<PairedListWidget*>(editExtraWidget),SIGNAL(valueChanged()),
            this, SLOT(emitValueChanged()));
    connect(static_cast<PairedListWidget*>(editExtraWidget),SIGNAL(valueChanged()),
            this, SLOT(updateEditDisplayWidget()));

    rawEditModeButton->setEnabled(true);
    rawEditModeButton->setChecked(false);
    setRawEditModeOff();
}

void PlotSettingsMultipleChoiceWidget::setRawEditModeOn()
{
    PlotSettingsBaseWidget::setRawEditModeOn();
    if (editExtraWidget)
        editExtraWidget->hide();
}

void PlotSettingsMultipleChoiceWidget::setRawEditModeOff()
{
    PlotSettingsBaseWidget::setRawEditModeOff();
    if (editExtraWidget)
        editExtraWidget->show();
}



void PlotSettingsMultipleChoiceWidget::updateEditDisplayWidget()
{
    QDomElement typeElement = XMLAccessor::childElement(settingsElement, "type");
    QString displayValue = (XMLAccessor::value(typeElement)) == "factor" ?
                getWidgetValue().toStringList().replaceInStrings(QRegExp("^|$"),"\"").join(", ") :
                getWidgetValue().toStringList().join(" ");
    static_cast<QLineEdit*>(editDisplayWidget)->setText(displayValue);
}

void PlotSettingsMultipleChoiceWidget::resetDefault()
{
    PlotSettingsBaseWidget::resetDefault();
    if (editMode == WidgetEditMode)
        updateEditDisplayWidget();
}


QVariant PlotSettingsMultipleChoiceWidget::getWidgetValue()
{
    if (editExtraWidget)
        return static_cast<PairedListWidget*>(editExtraWidget)->getValue();

    return QStringList();
}

void PlotSettingsMultipleChoiceWidget::setWidgetValue(QVariant val)
{
    static_cast<PairedListWidget*>(editExtraWidget)->setValue(val.toStringList());
    updateEditDisplayWidget();
}

QVariant PlotSettingsMultipleChoiceWidget::raw2widgetValue(QString val)
{
    if (val.isEmpty() || val == "NULL")
        return QStringList();

    QDomElement typeElement = XMLAccessor::childElement(settingsElement, "type");
    if ((XMLAccessor::value(typeElement)) == "factor" && useRFormat)
        return val.remove(QRegExp("^\\s*c\\(|\\)\\s*$|\"")).simplified().split(QRegExp("\\s*,\\s*")); // |^\\s*NULL\\s*$

    return BracketedParser::parse(val);
}

QString PlotSettingsMultipleChoiceWidget::widget2rawValue(QVariant val)
{
    QDomElement typeElement = XMLAccessor::childElement(settingsElement, "type");
    QDomElement defaultElement = XMLAccessor::childElement(settingsElement, "default");
    QStringList stringListVal = val.toStringList();
    if (stringListVal.isEmpty())
    {
        if (!hasDefault())
            return QString();
        else if (XMLAccessor::value(defaultElement) == "NULL")
            return "NULL";
    }

    return ((XMLAccessor::value(typeElement)) == "factor" && useRFormat) ?
        QString("c(%1)").arg(stringListVal.replaceInStrings(QRegExp("^|$"),"\"").join(", ")) :
        stringListVal.join(' ');
}
