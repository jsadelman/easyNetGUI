#include <QtWidgets>

#include "plotsettingsbasewidget.h"
//#include "lazynutjobparam.h"
//#include "sessionmanager.h"
//#include "lazynutlistwidget.h"
#include "lazynutlistcombobox.h"
#include "lazynutpairedlistwidget.h"
#include "bracketedparser.h"


PlotSettingsBaseWidget::PlotSettingsBaseWidget(XMLelement settingsElement, QWidget *parent)
    : settingsElement(settingsElement), QFrame(parent)
{
    createDisplay();
    editMode = RawEditMode;

    // debug
//    connect(this, &PlotSettingsBaseWidget::valueChanged, [=](){
//        qDebug() << "valueChanged emitted, value(): " << value();});
}



void PlotSettingsBaseWidget::createDisplay()
{
//    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
//    setFrameShadow(QFrame::Sunken);
//    setFrameShape(QFrame::Panel);
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
//    editStackedLayout = new QStackedLayout;
//    editStackedWidget = new QStackedWidget;
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


    connect(rawEdit, SIGNAL(editingFinished()), this, SLOT(emitValueChanged()));
//    editStackedWidget->addWidget(rawEdit);
//    editStackedWidget->setCurrentIndex(editStackedWidget->indexOf(rawEdit));
    commentLabel = new QLabel;
    commentLabel->setText(settingsElement["comment"]());
    commentLabel->setWordWrap(true);
    commentLabel->setStyleSheet("QLabel {"
                                "background-color: lightyellow;"
                                "border: 1px solid black;"
                                "padding: 4px;"
//                                "font-family: Arial;"
                                "font-style: italic;"
                                "}");
//    editModeButtonBox = new QGroupBox("Edit mode");
//    editModeButtonBoxLayout = new QVBoxLayout;
//    debugButton = new QPushButton("value()");
//    connect(debugButton, &QPushButton::clicked, [=](){qDebug() << value();});
//    debugButton->setToolTip("Open edit window");
//    debugButton->setEnabled(false);
//    widgetEditButton  = new QRadioButton("Quick input");
//    widgetEditButton->setEnabled(false);
//    connect(widgetEditButton, SIGNAL(toggled(bool)), this, SLOT(setWidgetMode(bool)));
    rawEditModeButton = new QRadioButton("Manual input");
    rawEditModeButton->setEnabled(false);
    rawEditModeButton->setChecked(true);
    connect(rawEditModeButton, SIGNAL(clicked(bool)), this, SLOT(setRawEditMode(bool)));

//    editModeButtonBoxLayout->addWidget(widgetEditButton);
//    editModeButtonBoxLayout->addWidget(editModeButton);
//    editModeButtonBox->setLayout(editModeButtonBoxLayout);


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
//    gridLayout->addWidget(editStackedWidget, 0, 1);

    gridLayout->addWidget(defaultButton, 0, 2);
    gridLayout->addWidget(commentLabel, 1, 0, 1, 2);
    gridLayout->addWidget(rawEditModeButton, 1, 2);
//    gridLayout->addWidget(editModeButtonBox, 0, 3, 2, 1);
//    gridLayout->setRowStretch(0,1);
//    gridLayout->setRowStretch(1,1);
    vboxLayout = new QVBoxLayout;
    vboxLayout->addLayout(gridLayout);
    vboxLayout->addWidget(rawEdit);
    rawEdit->hide();
    vboxLayout->addStretch();
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

void PlotSettingsBaseWidget::updateWidget(XMLelement xml)
{
    settingsElement = xml;
}


void PlotSettingsBaseWidget::resetDefault()
{
    QString defaultValue = hasDefault() ? settingsElement["default"]() : QString();
    if (valueSet)
    {
        setValue(defaultValue);
        qDebug() << " PlotSettingsBaseWidget::resetDefault() setValue. currentValue(), getValue()" << currentValue << getValue() ;

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
    editDisplayWidget->hide();
    editMode = RawEditMode;
}

void PlotSettingsBaseWidget::setRawEditModeOff()
{
    rawEdit->hide();
    editDisplayWidget->show();
    editMode = WidgetEditMode;
}


void PlotSettingsBaseWidget::emitValueChanged()
{
    if (currentValue != getValue())
    {
        currentValue = getValue();
        valueSet = true;
        qDebug() << "emitValueChanged: currentValue: " << currentValue << "value(): " << value();
        emit valueChanged();
    }
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
        connect(static_cast<QDoubleSpinBox*>(editDisplayWidget), SIGNAL(editingFinished()),
                this, SLOT(emitValueChanged()));
    }
    else
    {
        editDisplayWidget = new QSpinBox;
        setWidgetValue(settingsElement["value"]());
        connect(static_cast<QSpinBox*>(editDisplayWidget), SIGNAL(editingFinished()),
                this, SLOT(emitValueChanged()));
    }
    currentValue = settingsElement["value"]();
    valueSet = !settingsElement["value"]().isEmpty();
//    editStackedWidget->addWidget(editDisplayWidget);
    gridLayout->addWidget(editDisplayWidget, 0, 1);
//    widgetEditButton->setEnabled(true);
    rawEditModeButton->setEnabled(true);
    rawEditModeButton->setChecked(false);
    setRawEditModeOff();
}



//////////// PlotSettingsSingleChoiceWidget



PlotSettingsSingleChoiceWidget::PlotSettingsSingleChoiceWidget(XMLelement settingsElement, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, parent)
{
    createEditWidget();
}

void PlotSettingsSingleChoiceWidget::updateWidget(XMLelement xml)
{
    PlotSettingsBaseWidget::updateWidget(xml);
    delete editDisplayWidget;
    createEditWidget();
}


void PlotSettingsSingleChoiceWidget::createEditWidget()
{
    editDisplayWidget = new LazyNutListComboBox(settingsElement["levels"](), this);
    connect(static_cast<LazyNutListComboBox*>(editDisplayWidget),SIGNAL(listReady()),
            this, SLOT(setupEditWidget()));
    static_cast<LazyNutListComboBox*>(editDisplayWidget)->getList();
}

void PlotSettingsSingleChoiceWidget::setupEditWidget()
{
    if (!hasDefault())
        static_cast<LazyNutListComboBox*>(editDisplayWidget)->setEmptyItem(true);

    setWidgetValue(raw2widgetValue(settingsElement["value"]()));
    currentValue = settingsElement["value"]();
    valueSet = !settingsElement["value"]().isEmpty();
    connect(static_cast<LazyNutListComboBox*>(editDisplayWidget),SIGNAL(activated(int)),
            this, SLOT(emitValueChanged()));
//    editStackedWidget->addWidget(static_cast<LazyNutListComboBox*>(editDisplayWidget));
    gridLayout->addWidget(editDisplayWidget, 0, 1);
    if (settingsElement["levels"]().isEmpty())
    {
        rawEditModeButton->setEnabled(false);
//        widgetEditButton->setEnabled(false);
        rawEditModeButton->setChecked(true);
        setRawEditModeOn();
    }
    else
    {
//        widgetEditButton->setEnabled(true);
        rawEditModeButton->setEnabled(true);
        rawEditModeButton->setChecked(false);
        setRawEditModeOff();
//        widgetEditButton->setChecked(true);
    }
    valueSet = !settingsElement["value"]().isEmpty();
    currentValue = value();
}

QVariant PlotSettingsSingleChoiceWidget::getWidgetValue()
{
    return static_cast<QComboBox*>(editDisplayWidget)->currentText();
}

void PlotSettingsSingleChoiceWidget::setWidgetValue(QVariant val)
{
    static_cast<QComboBox*>(editDisplayWidget)->setCurrentIndex(
                static_cast<QComboBox*>(editDisplayWidget)->findText(val.toString()));
}

QVariant PlotSettingsSingleChoiceWidget::raw2widgetValue(QString val)
{
    if (settingsElement["type"]() == "factor")
        return val.remove(QRegExp("^\\s*c\\(|\\)\\s*$|\"|^\\s*NULL\\s*$")).simplified();

    return val;
}

QString PlotSettingsSingleChoiceWidget::widget2rawValue(QVariant val)
{
    QString item = val.toString();
    if (settingsElement["type"]() == "factor")
    {
        if (!item.isEmpty())
            item = QString("\"%1\"").arg(item);

        return  QString("c(%1)").arg(item);
    }
    return item;
}

//////////////////////// PlotSettingsMultipleChoiceWidget


PlotSettingsMultipleChoiceWidget::PlotSettingsMultipleChoiceWidget(XMLelement settingsElement, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, parent)
{
    createEditWidget();
}

void PlotSettingsMultipleChoiceWidget::updateWidget(XMLelement xml)
{
    PlotSettingsBaseWidget::updateWidget(xml);
    delete editDisplayWidget;
    delete editExtraWidget;
    createEditWidget();
}

void PlotSettingsMultipleChoiceWidget::setupEditWidget()
{
    setWidgetValue(raw2widgetValue(settingsElement["value"]()));
    updateEditDisplayWidget();
    currentValue = settingsElement["value"]();
    valueSet = !settingsElement["value"]().isEmpty();

    connect(static_cast<LazyNutPairedListWidget*>(editExtraWidget),SIGNAL(valueChanged()),
            this, SLOT(emitValueChanged()));
    connect(static_cast<LazyNutPairedListWidget*>(editExtraWidget),SIGNAL(valueChanged()),
            this, SLOT(updateEditDisplayWidget()));
//    editStackedWidget->addWidget(editDisplayWidget);
    gridLayout->addWidget(editDisplayWidget, 0,1);

    if (settingsElement["levels"]().isEmpty())
    {
        rawEditModeButton->setEnabled(false);
//        widgetEditButton->setEnabled(false);
        rawEditModeButton->setChecked(true);
        setRawEditModeOn();
    }
    else
    {
//        widgetEditButton->setEnabled(true);
        rawEditModeButton->setEnabled(true);
        rawEditModeButton->setChecked(false);
        setRawEditModeOff();
//        widgetEditButton->setChecked(true);
    }
}

void PlotSettingsMultipleChoiceWidget::setRawEditModeOn()
{
    PlotSettingsBaseWidget::setRawEditModeOn();
    editExtraWidget->hide();
}

void PlotSettingsMultipleChoiceWidget::setRawEditModeOff()
{
    PlotSettingsBaseWidget::setRawEditModeOff();
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
    updateEditDisplayWidget();
}

void PlotSettingsMultipleChoiceWidget::createEditWidget()
{
    editDisplayWidget = new QLineEdit();
    static_cast<QLineEdit*>(editDisplayWidget)->setReadOnly(true);

    editExtraWidget = new LazyNutPairedListWidget(settingsElement["levels"]());
    connect(static_cast<LazyNutPairedListWidget*>(editExtraWidget),SIGNAL(listReady()),
            this, SLOT(setupEditWidget()));
//    connect(static_cast<LazyNutPairedListWidget*>(editExtraWidget),SIGNAL(sizeChanged()),
//            this, SIGNAL(sizeChanged()));
    static_cast<LazyNutPairedListWidget*>(editExtraWidget)->getList();
//    gridLayout->addWidget(editExtraWidget, 2, 0, 1, 3);
    vboxLayout->addWidget(editExtraWidget);
}

QVariant PlotSettingsMultipleChoiceWidget::getWidgetValue()
{
    return static_cast<LazyNutPairedListWidget*>(editExtraWidget)->getValue();
}

void PlotSettingsMultipleChoiceWidget::setWidgetValue(QVariant val)
{
    static_cast<LazyNutPairedListWidget*>(editExtraWidget)->setValue(val.toStringList());
    updateEditDisplayWidget();
}

QVariant PlotSettingsMultipleChoiceWidget::raw2widgetValue(QString val)
{
    if (settingsElement["type"]() == "factor")
        return val.remove(QRegExp("^\\s*c\\(|\\)\\s*$|\"|^\\s*NULL\\s*$")).simplified().split(QRegExp("\\s*,\\s*"));

    return BracketedParser::parse(val);
}

QString PlotSettingsMultipleChoiceWidget::widget2rawValue(QVariant val)
{
    if (settingsElement["type"]() == "factor")
        return QString("c(%1)").arg(val.toStringList().replaceInStrings(QRegExp("^|$"),"\"").join(", "));

    return val.toStringList().join(' ');
}
