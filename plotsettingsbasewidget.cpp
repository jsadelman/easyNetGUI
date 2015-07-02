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
#include <QMetaObject>


PlotSettingsBaseWidget::PlotSettingsBaseWidget(XMLelement settingsElement, QWidget *parent)
    : settingsElement(settingsElement), levelsListModel(nullptr), levelsCmdObjectWatcher(nullptr), editDisplayWidget(nullptr), QFrame(parent)
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
    QString labelText = settingsElement.label();
    if (!hasDefault())
        labelText.append("*");

    nameLabel->setText(labelText);
    nameLabel->setStyleSheet("QLabel {"
                             "font-weight: bold;"
                             "}");
    rawEdit = new QLineEdit;
    if (!hasDefault())
        rawEdit->setPlaceholderText("*mandatory field*");

    if (settingsElement["value"]().isEmpty())
    {
        if (hasDefault())
            rawEdit->setText(settingsElement["default"]());
    }
     else
        rawEdit->setText(settingsElement["value"]());


    connect(rawEdit, SIGNAL(textChanged(QString)), this, SLOT(emitValueChanged()));
    commentLabel = new QLabel;
    commentLabel->setText(settingsElement["comment"]());
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
        defaultButton->setToolTip(QString("Reset default value: %1").arg(settingsElement["default"]()));
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
                settingsElement["value"]() != settingsElement["default"]() :
                !settingsElement["value"]().isEmpty();

    currentValue = settingsElement["value"]();


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

    XMLelement levelsElement = settingsElement["levels"];
    if (levelsElement.isNull())
        return;
    else if (levelsElement.isParameter())
    {
        levelsListModel = new ObjectCatalogueFilter(this);
        static_cast<ObjectCatalogueFilter*>(levelsListModel)->setType(levelsElement.type());

    }
    else if (levelsElement.isCommand())
    {
        levelsListModel = new StringListModel(QStringList(), this);
        levelsCmdObjectWatcher = new ObjectCatalogueFilter(this);
        QStringList objectsInCmd;
        XMLelement cmdElement = levelsElement.firstChild("object");
        while (!cmdElement.isNull())
        {
            objectsInCmd.append(cmdElement());
            cmdElement = cmdElement.nextSibling("object");
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
    return settingsElement.listLabels().contains("default");
}


QString PlotSettingsBaseWidget::value()
{
    if (isValueSet())
        return getValue();

    return QString();
}

QString PlotSettingsBaseWidget::settingMethod()
{
    return settingsElement["default"]() == "dataframe" ? "setting_object" : "setting";
}

void PlotSettingsBaseWidget::updateWidget(XMLelement xml)
{
    settingsElement = xml;
    if (!levelsListModel || settingsElement["levels"].isNull())
        createLevelsListModel();
}


void PlotSettingsBaseWidget::resetDefault()
{
    QString defaultValue = hasDefault() ? settingsElement["default"]() : QString();
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
        emit valueChanged();
    }
}

void PlotSettingsBaseWidget::getLevels()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->cmdList = QStringList({settingsElement["levels"]().prepend("xml ")});
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(qobject_cast<StringListModel*>(levelsListModel), SLOT(updateList(QStringList)));
    param->setEndOfJobReceiver(this, SIGNAL(levelsReady()));
    SessionManager::instance()->setupJob(param, sender());
}


//////////// PlotSettingsNumericWidget



PlotSettingsNumericWidget::PlotSettingsNumericWidget(XMLelement settingsElement, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, parent)
{
    createEditWidget();
}

void PlotSettingsNumericWidget::setWidgetValue(QVariant val)
{
    if (settingsElement["value"].isReal())
        static_cast<QDoubleSpinBox*>(editDisplayWidget)->setValue(val.toDouble());
    else
        static_cast<QSpinBox*>(editDisplayWidget)->setValue(val.toInt());
}


QVariant PlotSettingsNumericWidget::getWidgetValue()
{
    if (settingsElement["value"].isReal())
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
    if (settingsElement["value"].isReal())
    {
        editDisplayWidget = new QDoubleSpinBox;
        setWidgetValue(settingsElement["value"]());
        connect(static_cast<QDoubleSpinBox*>(editDisplayWidget), SIGNAL(valueChanged(double)),
                this, SLOT(emitValueChanged()));
    }
    else
    {
        editDisplayWidget = new QSpinBox;
        setWidgetValue(settingsElement["value"]());
        connect(static_cast<QSpinBox*>(editDisplayWidget), SIGNAL(valueChanged(int)),
                this, SLOT(emitValueChanged()));
    }
    currentValue = settingsElement["value"]();
    valueSet = !settingsElement["value"]().isEmpty();
    gridLayout->addWidget(editDisplayWidget, 0, 1);
    rawEditModeButton->setEnabled(true);
    rawEditModeButton->setChecked(false);
    setRawEditModeOff();
}



//////////// PlotSettingsSingleChoiceWidget



PlotSettingsSingleChoiceWidget::PlotSettingsSingleChoiceWidget(XMLelement settingsElement, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, parent)
{
    connect(this, SIGNAL(levelsReady()), this, SLOT(buildEditWidget()));
    createEditWidget();
}

void PlotSettingsSingleChoiceWidget::updateWidget(XMLelement xml)
{
    PlotSettingsBaseWidget::updateWidget(xml);
    delete editDisplayWidget;
    editDisplayWidget = nullptr;
    createEditWidget();
}


void PlotSettingsSingleChoiceWidget::createEditWidget()
{
    XMLelement levelsElement = settingsElement["levels"];

    if (levelsElement.isParameter())
        buildEditWidget();
    else if (levelsElement.isCommand())
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
    editDisplayWidget = new QComboBox;
    static_cast<QComboBox*>(editDisplayWidget)->setModel(levelsListModel);
    setWidgetValue(raw2widgetValue(settingsElement["value"]()));
    currentValue = settingsElement["value"]();
    valueSet = !settingsElement["value"]().isEmpty();
    connect(static_cast<QComboBox*>(editDisplayWidget),SIGNAL(currentIndexChanged(int)),
            this, SLOT(emitValueChanged()));
    gridLayout->addWidget(editDisplayWidget, 0, 1);
    rawEditModeButton->setEnabled(true);
    rawEditModeButton->setChecked(false);
    setRawEditModeOff();
    valueSet = !settingsElement["value"]().isEmpty();
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

    return settingsElement["type"]() == "factor" ?
        val.remove(QRegExp("^\\s*c\\(|\\)\\s*$|\"")).simplified() : val;
}

QString PlotSettingsSingleChoiceWidget::widget2rawValue(QVariant val)
{
    QString stringVal = val.toString();
    if (stringVal.isEmpty())
    {
        if (!hasDefault())
            return QString();
        else if (settingsElement["default"]() == "NULL")
            return "NULL";
    }
    return settingsElement["type"]() == "factor" ?
            QString("c(\"%1\")").arg(stringVal) : stringVal;
}

//////////////////////// PlotSettingsMultipleChoiceWidget


PlotSettingsMultipleChoiceWidget::PlotSettingsMultipleChoiceWidget(XMLelement settingsElement, QWidget *parent)
    : editExtraWidget(nullptr), PlotSettingsBaseWidget(settingsElement, parent)
{
    connect(this, SIGNAL(levelsReady()), this, SLOT(buildEditWidget()));
    createEditWidget();
}

void PlotSettingsMultipleChoiceWidget::updateWidget(XMLelement xml)
{
    PlotSettingsBaseWidget::updateWidget(xml);
    delete editDisplayWidget;
    editDisplayWidget = nullptr;
    delete editExtraWidget;
    editExtraWidget = nullptr;
    createEditWidget();
}


void PlotSettingsMultipleChoiceWidget::createEditWidget()
{
    XMLelement levelsElement = settingsElement["levels"];

    if (levelsElement.isParameter())
        buildEditWidget();
    else if (levelsElement.isCommand())
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
    editDisplayWidget = new QLineEdit();
    static_cast<QLineEdit*>(editDisplayWidget)->setReadOnly(true);
    gridLayout->addWidget(editDisplayWidget, 0,1);
    editExtraWidget = new PairedListWidget(levelsListModel);

    vboxLayout->addWidget(editExtraWidget);

    setWidgetValue(raw2widgetValue(settingsElement["value"]()));
    updateEditDisplayWidget();
    currentValue = settingsElement["value"]();
    valueSet = !settingsElement["value"]().isEmpty();

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
    QString displayValue = settingsElement["type"]() == "factor" ?
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

    if (settingsElement["type"]() == "factor")
        return val.remove(QRegExp("^\\s*c\\(|\\)\\s*$|\"")).simplified().split(QRegExp("\\s*,\\s*")); // |^\\s*NULL\\s*$

    return BracketedParser::parse(val);
}

QString PlotSettingsMultipleChoiceWidget::widget2rawValue(QVariant val)
{
    QStringList stringListVal = val.toStringList();
    if (stringListVal.isEmpty())
    {
        if (!hasDefault())
            return QString();
        else if (settingsElement["default"]() == "NULL")
            return "NULL";
    }

    return (settingsElement["type"]() == "factor") ?
        QString("c(%1)").arg(stringListVal.replaceInStrings(QRegExp("^|$"),"\"").join(", ")) :
        stringListVal.join(' ');
}
