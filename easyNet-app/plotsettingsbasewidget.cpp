#include <QtWidgets>
#include <QMetaObject>
#include <QSortFilterProxyModel>
#include <QFontMetrics>
#include <QFont>

#include "plotsettingsbasewidget.h"
#include "lazynutjob.h"
#include "sessionmanager.h"
#include "pairedlistwidget.h"
#include "bracketedparser.h"
#include "simplelistmodel.h"
#include "selectfromlistmodel.h"
#include "objectcachefilter.h"
#include "objectcache.h"
#include "proxymodelextrarows.h"
#include "xmlaccessor.h"
#include "easyNetMainWindow.h"



PlotSettingsBaseWidget::PlotSettingsBaseWidget(QDomElement& settingsElement, bool _useRFormat, QWidget *parent)
    : settingsElement(settingsElement),
      useRFormat(_useRFormat),
      levelsListModel(nullptr),
      levelsCmdObjectWatcher(nullptr),
      editDisplayWidget(nullptr),
      descriptionUpdater(nullptr),
      currentValue(),
      QFrame(parent)
{
    createDisplay();
    createLevelsListModel();
    editMode = RawEditMode;
    QDomElement settingsUseRFormat = XMLAccessor::childElement(settingsElement, "useRFormat");
    if (!settingsUseRFormat.isNull())
        useRFormat = XMLAccessor::value(settingsUseRFormat) == "true";
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
//    rawEdit->hide();
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
    levelsDescriptionElement.clear();
    objectsInCmd.clear();
    if(descriptionUpdater)
    {
        delete descriptionUpdater;
        descriptionUpdater = nullptr;
    }
    QSortFilterProxyModel *proxy = qobject_cast<QSortFilterProxyModel *>(levelsListModel);
    if (proxy)
        delete proxy->sourceModel();
    delete levelsListModel;
    levelsListModel = nullptr;

    QDomElement levelsElement = XMLAccessor::childElement(settingsElement, "levels");
    if (levelsElement.isNull())
    {
        return;
    }
    else if (levelsElement.tagName() == "parameter")
    {
        levelsListModel = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
        static_cast<ObjectCacheFilter*>(levelsListModel)->setType(XMLAccessor::type(levelsElement));

    }
    else if (levelsElement.tagName() == "list")
    {
        levelsListModel = new StringListModel(XMLAccessor::listValues(levelsElement), this);
    }
    else if (levelsElement.tagName() == "command")
    {
        StringListModel *stringListModel = new StringListModel(QStringList(), this);
        levelsListModel = new QSortFilterProxyModel(this);
        static_cast<QSortFilterProxyModel *>(levelsListModel)->setSourceModel(stringListModel);
        levelsCmdObjectWatcher = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);

        QDomElement cmdElement = levelsElement.firstChildElement("object");
        while (!cmdElement.isNull())
        {
            objectsInCmd.append(XMLAccessor::value(cmdElement));
            cmdElement = cmdElement.nextSiblingElement("object");
        }
        bool descrel=false;
        cmdElement = levelsElement.firstChildElement();
        if(cmdElement.tagName()=="object")
        {
            cmdElement = cmdElement.nextSiblingElement();
            if(XMLAccessor::value(cmdElement)=="get_description_element")
            {
                descrel=true;
                cmdElement = cmdElement.nextSiblingElement();
                levelsDescriptionElement = XMLAccessor::value(cmdElement);
            }
        }
        levelsCmdObjectWatcher->setNameList(objectsInCmd);
        if(!descrel)
        {
            bool identity = true;
            QDomElement dependenciesElement = XMLAccessor::childElement(settingsElement, "dependencies");
            QDomElement depValElement = dependenciesElement.firstChildElement();
            while (!depValElement.isNull())
            {
                identity = identity && XMLelement(depValElement)["type"]() == "identity";
                depValElement = depValElement.nextSiblingElement();
            }
            if (!identity)
                connect(levelsCmdObjectWatcher, SIGNAL(objectModified(QString)), this, SLOT(getLevels()));

            connect(levelsCmdObjectWatcher, &ObjectCacheFilter::objectDestroyed, [=]()
            {
                 static_cast<StringListModel *>(static_cast<QSortFilterProxyModel*>(levelsListModel)->sourceModel())->updateList(QStringList());
            });
        }
        else
        {
            descriptionUpdater = new ObjectUpdater(this);
            descriptionUpdater->setCommand("description");
            connect(descriptionUpdater, SIGNAL(objectUpdated(QDomDocument*, QString)),
                    this, SLOT(getLevelsFromDescription(QDomDocument*,QString)));
            descriptionUpdater->setProxyModel(levelsCmdObjectWatcher);

//            connect(levelsCmdObjectWatcher, SIGNAL(objectModified(QString)), this, SLOT(getLevelsDescriptionReady(QString)));
        }
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
    XMLAccessor::setValue(valueElement, val);

    switch(editMode)
    {
    case RawEditMode:
    {
        rawEdit->setText(val);
        break;
    }
    case WidgetEditMode:
    {
        setWidgetValue(raw2widgetValue(val));
    }
    }
}

bool PlotSettingsBaseWidget::isDataframe()
{
    QDomElement typeElement = XMLAccessor::childElement(settingsElement, "type");
    return XMLAccessor::value(typeElement) == "dataframe";
}

bool PlotSettingsBaseWidget::isValueSet()
{
    return valueSet || rawEdit->isModified();
}

bool PlotSettingsBaseWidget::hasDefault()
{
    return (XMLAccessor::listLabels(settingsElement)).contains("default");
}

QString PlotSettingsBaseWidget::defaultValue()
{
    if (hasDefault())
    {
        QDomElement settingsDefault = XMLAccessor::childElement(settingsElement, "default");
        return XMLAccessor::value(settingsDefault);
    }
    return QString();
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
//    QDomElement levelsElement = XMLAccessor::childElement(settingsElement, "levels");
//    if (!levelsListModel || levelsElement.isNull())
//    {
        createLevelsListModel();
//    }
}


void PlotSettingsBaseWidget::resetDefault()
{
    QDomElement defaultElement = XMLAccessor::childElement(settingsElement, "default");
    QString defaultValue = hasDefault() ? XMLAccessor::value(defaultElement) : QString();
    if (valueSet)
    {
        setValue(defaultValue);
        valueSet = false;
        QString newValue = getValue();
        if (currentValue != newValue)
        {
            emit valueChanged(currentValue, newValue);
            currentValue = newValue;
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
    {
        editDisplayWidget->hide();
    }
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
    QString newValue = getValue();
    if (currentValue != newValue)
    {
        QDomElement valueElement = XMLAccessor::childElement(settingsElement, "value");
        XMLAccessor::setValue(valueElement, newValue);
        valueSet = true;
        emit valueChanged(currentValue, newValue);
        currentValue = newValue;
    }
}

void PlotSettingsBaseWidget::getLevels()
{
    if(levelsDescriptionElement.length()>0)
    {
        for(auto x:objectsInCmd)
        {
          descriptionUpdater->requestObject(x);
        }
        return;
    }
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
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({(XMLAccessor::command(levelsElement)).prepend("xml ")});
    job->setAnswerReceiver(qobject_cast<StringListModel*>(qobject_cast<QSortFilterProxyModel*>(levelsListModel)->sourceModel()),
                           SLOT(updateList(QStringList)), AnswerFormatterType::ListOfValues);
    job->appendEndOfJobReceiver(this, SIGNAL(levelsReady()));
    SessionManager::instance()->submitJobs(job);
}

void PlotSettingsBaseWidget::getLevelsDescriptionReady(QString name)
{
    getLevelsFromDescription(SessionManager::instance()->descriptionCache->getDomDoc(name),name);
}

void PlotSettingsBaseWidget::getLevelsFromDescription(QDomDocument *desc, QString)
{
    QStringList levels;
    if(!desc) return;
    QDomElement level = desc->documentElement();
    level = XMLAccessor::childElement(level,levelsDescriptionElement).firstChildElement();
    while(!level.isNull())
    {
        levels.append(XMLAccessor::value(level));
        level=level.nextSiblingElement();
    }
    qobject_cast<StringListModel*>(qobject_cast<QSortFilterProxyModel*>(levelsListModel)->sourceModel())
             ->updateList(levels);
    emit levelsReady();
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
        static_cast<QDoubleSpinBox*>(editDisplayWidget)->setMaximum(10000);
        static_cast<QDoubleSpinBox*>(editDisplayWidget)->setMinimum(-10);
        setWidgetValue(XMLAccessor::value(valueElement));
        connect(static_cast<QDoubleSpinBox*>(editDisplayWidget), SIGNAL(valueChanged(double)),
                this, SLOT(emitValueChanged()));
    }
    else
    {
        editDisplayWidget = new QSpinBox;
        static_cast<QSpinBox*>(editDisplayWidget)->setMaximum(10000);
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

    if (levelsElement.tagName() == "parameter" )
        buildEditWidget();
    else if (levelsElement.tagName() == "command")
        getLevels();
    else if (levelsElement.tagName() == "list")
    {
        buildEditWidget();
    }
    else
    {
        delete editDisplayWidget;
        editDisplayWidget = nullptr;
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
    levelsListModel->sort(0);
    static_cast<QComboBox*>(editDisplayWidget)->setModel(levelsListModel);

    QDomElement valueElement = XMLAccessor::childElement(settingsElement, "value");
    currentValue = XMLAccessor::value(valueElement);

    setWidgetValue(raw2widgetValue(currentValue));
     valueSet = !currentValue.isEmpty();

//    connect(static_cast<QComboBox*>(editDisplayWidget),SIGNAL(currentIndexChanged(int)),
    connect(static_cast<QComboBox*>(editDisplayWidget),SIGNAL(currentIndexChanged(int)),
            this, SLOT(emitValueChanged()));


//    connect(static_cast<QComboBox*>(editDisplayWidget),SIGNAL(currentTextChanged(QString)),
//            this, SLOT(emitValueChanged()));
    connect(static_cast<QComboBox*>(editDisplayWidget),  static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),
            [=](QString txt){
//        qDebug() << "PlotSettingsSingleChoiceWidget text changed" << txt << name();
    });
//    qDebug () << Q_FUNC_INFO << name() << currentValue << value();
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
    else if (levelsElement.tagName() == "list")
        buildEditWidget();
    else if (levelsElement.tagName() == "command")
        getLevels();
    else
    {
        delete editDisplayWidget;
        editDisplayWidget = nullptr;
        delete editExtraWidget;
        editExtraWidget = nullptr;
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
    currentValue = value();
    currentValue = (currentValue.isEmpty() || currentValue == "c()") ? XMLAccessor::value(valueElement) : currentValue;
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
    {
        return val.remove(QRegExp("^\\s*c\\(|\\)\\s*$|\"")).simplified().split(QRegExp("\\s*,\\s*")); // |^\\s*NULL\\s*$
    }


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


/////////////////////////// PlotSettingsFilenameWidget

PlotSettingsFilenameWidget::PlotSettingsFilenameWidget(QDomElement &settingsElement, bool useRFormat, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, useRFormat, parent)
{
    createEditWidget();
}

void PlotSettingsFilenameWidget::createEditWidget()
{
    QWidget *editDisplayWidgetContainer = new QWidget(this);
    QHBoxLayout *editDisplayWidgetContainerLayout = new QHBoxLayout;
    editDisplayWidget = new QLineEdit();
    static_cast<QLineEdit*>(editDisplayWidget)->setReadOnly(true);
    editDisplayWidgetContainerLayout->addWidget(editDisplayWidget);
    QPushButton *browseButton = new QPushButton("Browse");
    connect(browseButton,  SIGNAL(clicked()), this, SLOT(browseFilename()));
    editDisplayWidgetContainerLayout->addWidget(browseButton);
    editDisplayWidgetContainer->setLayout(editDisplayWidgetContainerLayout);
    gridLayout->addWidget(editDisplayWidgetContainer, 0, 1);
    QDomElement valueElement = XMLAccessor::childElement(settingsElement, "value");
    setWidgetValue(XMLAccessor::value(valueElement));
    connect(static_cast<QLineEdit*>(editDisplayWidget), SIGNAL(textChanged(QString)),
            this, SLOT(emitValueChanged()));
    currentValue = XMLAccessor::value(valueElement);
    valueSet = !currentValue.isEmpty();
    rawEditModeButton->setEnabled(true);
    rawEditModeButton->setChecked(false);
    setRawEditModeOff();
}

void PlotSettingsFilenameWidget::browseFilename()
{
    QFileInfo defaultFullPathInfo(QString("%1/%2").arg(SessionManager::instance()->easyNetDataHome()).arg(defaultValue()));
    QString fileName = QFileDialog::getOpenFileName(this, name(),
                                                    defaultFullPathInfo.absoluteFilePath(),
                                                    QString("(*.%1)").arg(defaultFullPathInfo.suffix()));
    setValue(QDir(SessionManager::instance()->easyNetDataHome()).relativeFilePath(fileName));

}

QVariant PlotSettingsFilenameWidget::getWidgetValue()
{
    return static_cast<QLineEdit *>(editDisplayWidget)->text();
}

void PlotSettingsFilenameWidget::setWidgetValue(QVariant val)
{
    static_cast<QLineEdit *>(editDisplayWidget)->setText(val.toString());
    // resize to content
    int pixelsWide = qApp->fontMetrics().width(static_cast<QLineEdit *>(editDisplayWidget)->text()) + 10;
    static_cast<QLineEdit*>(editDisplayWidget)->setMinimumWidth(pixelsWide);
    adjustSize();
}

QVariant PlotSettingsFilenameWidget::raw2widgetValue(QString val)
{
    return val;
}

QString PlotSettingsFilenameWidget::widget2rawValue(QVariant val)
{
    return val.toString();
}

