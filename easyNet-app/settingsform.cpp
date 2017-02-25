#include "settingsform.h"
#include "plotsettingsbasewidget.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"

#include <QDomDocument>
#include <QVBoxLayout>
#include <QMap>
#include <QMetaObject>
#include <QDebug>
#include <QVBoxLayout>
#include <QComboBox>

SettingsForm::SettingsForm(QDomDocument *domDoc, QWidget *parent)
   : domDoc(domDoc), m_useRFormat(false), QTabWidget(parent)
{
    rootElement = domDoc->documentElement();
}

SettingsForm::~SettingsForm()
{
    delete domDoc;
}


void SettingsForm::build()
{
    QDomElement domElement = rootElement.firstChildElement();
    while (!domElement.isNull())
    {
        PlotSettingsBaseWidget *widget = createWidget(domElement);
        widgetMap.insert(widget->name(), widget);
        hasChanged.insert(widget->name(), false);
        QDomElement groupingElement = XMLAccessor::childElement(domElement, "grouping");
        QString tabname = XMLAccessor::value(groupingElement);
        QWidget* tab = 0;
        QVBoxLayout* lay = 0;
        bool novel=!twidgetMap.contains(tabname);
        if(novel)
        {
          tabOrder.append(tabname);
          twidgetMap[tabname] = new QWidget;
          layoutMap[tabname] = new QVBoxLayout;
        }
        tab=twidgetMap[tabname];
        lay=layoutMap[tabname];
        lay->addWidget(widget);

        //        mainLayout->addWidget(widget);
        //addTab(widget, settingsElement.label());
        domElement = domElement.nextSiblingElement();
    }

    for(auto tabname: tabOrder)
    {
        layoutMap[tabname]->addStretch();
        layoutMap[tabname]->setSizeConstraint(QLayout::SetFixedSize);
        twidgetMap[tabname]->setLayout(layoutMap[tabname]);
        addTab(twidgetMap[tabname],tabname);
    }
    initDependersSet();
    QMap<QString, QString>::const_iterator i = m_defaultSettings.constBegin();
    while (i != m_defaultSettings.constEnd())
    {
        setSetting(i.key(), i.value());
        ++i;
    }
}

QMap<QString, QString> SettingsForm::getSettings()
{
    QMap<QString, QString> settings;
    foreach (QString setting, widgetMap.keys())
    {
        settings.insert(setting, widgetMap[setting]->value());
    }
    return settings;
}

void SettingsForm::initDependersSet()
{
    QDomElement settingsElement = rootElement.firstChildElement();
    while (!settingsElement.isNull())
    {
        QDomElement dependenciesElement = XMLAccessor::childElement(settingsElement, "dependencies");
        foreach (QString depender, XMLAccessor::listValues(dependenciesElement))
            dependersSet.insert(depender);

        settingsElement = settingsElement.nextSiblingElement();
    }
}



QStringList SettingsForm::getSettingsCmdList(bool force)
{
    QStringList cmdList;
    foreach (QString setting, XMLAccessor::listLabels(rootElement))
        if (hasChanged[setting] || force)
        {
            cmdList.append(getSettingCmdLine(setting));
            hasChanged[setting] = false;
        }
    return cmdList;
}

QString SettingsForm::value(QString label)
{
    return widgetMap[label]->value();
}


PlotSettingsBaseWidget *SettingsForm::createWidget(QDomElement &domElement)
{
    // TODO: implement this with a factory
    QDomElement typeElement = XMLAccessor::childElement(domElement, "type");
    QDomElement choiceElement = XMLAccessor::childElement(domElement, "choice");
    QDomElement defaultElement = XMLAccessor::childElement(domElement, "default");
    QString type = XMLAccessor::value(typeElement);
    QString choice = XMLAccessor::value(choiceElement);
    bool isObject = XMLAccessor::type(defaultElement)=="object";
    PlotSettingsBaseWidget *widget;
    if (type == "numeric")
        widget = new PlotSettingsNumericWidget(domElement, m_useRFormat);

    else if ((isObject || type == "factor") && choice == "single")
        widget = new PlotSettingsSingleChoiceWidget(domElement, m_useRFormat);

    else if ((isObject || type == "factor") && choice == "multiple")
        widget = new PlotSettingsMultipleChoiceWidget(domElement, m_useRFormat);

    else if (type == "filename")
        widget = new PlotSettingsFilenameWidget(domElement, m_useRFormat);

    else
        widget = new PlotSettingsBaseWidget(domElement, m_useRFormat);


    connect(widget, SIGNAL(valueChanged(QString, QString)), this, SLOT(recordValueChange(QString, QString)));
    connect(widget, SIGNAL(valueChanged(QString, QString)), this, SLOT(checkDependencies()));
//    connect(widget, SIGNAL(sizeChanged()), this, SLOT(updateSize()));
    return widget;
}

void SettingsForm::recordValueChange(QString oldValue, QString newValue)
{
    Q_UNUSED(oldValue)
    Q_UNUSED(newValue)
    PlotSettingsBaseWidget* widget = qobject_cast<PlotSettingsBaseWidget*>(sender());
    hasChanged[widget->name()] = true;
}

void SettingsForm::checkDependencies()
{
    PlotSettingsBaseWidget* widget = qobject_cast<PlotSettingsBaseWidget*>(sender());
    if (widget && dependersSet.contains(widget->name()))
    {
        dependerOnUpdate = widget->name();
        triggerUpdateDependees();
    }
}

void SettingsForm::updateDependees(QDomDocument* newDomDoc)
{
    if (newDomDoc)
    {
        delete domDoc;
        domDoc = newDomDoc;
        rootElement = domDoc->documentElement();
    }
    if (dependerOnUpdate.isEmpty())
        return; // just safety
    QDomElement settingsElement = rootElement.firstChildElement();
    while (!settingsElement.isNull())
    {
        QDomElement dependenciesElement = XMLAccessor::childElement(settingsElement, "dependencies");
        if ((XMLAccessor::listValues(dependenciesElement)).contains(dependerOnUpdate))
        {
            substituteDependentValues(settingsElement);
            widgetMap[XMLAccessor::label(settingsElement)]->updateWidget(settingsElement);
        }
        settingsElement = settingsElement.nextSiblingElement();
    }
}

void SettingsForm::updateSize()
{
    // http://doc.qt.digia.com/qq/qq06-qwidgetstack.html
    for (int i = 0; i < count(); ++i)
    {
        if (i == currentIndex())
            widget(i)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        else
            widget(i)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    }
//    layout()->activate();
    //    setFixedHeight(currentWidget()->minimumSizeHint().height());
}

void SettingsForm::triggerUpdateDependees()
{
    updateDependees();
}



QString SettingsForm::getSettingCmdLine(QString setting)
{
    QDomElement settingsElement = XMLAccessor::childElement(rootElement, setting);
    QDomElement typeElement = XMLAccessor::childElement(settingsElement, "type");
    QDomElement defaultElement = XMLAccessor::childElement(settingsElement, "default");
    return QString("%1 %2 %3 %4")
            .arg(m_name)
            .arg(XMLAccessor::type(defaultElement) == "object" ? "setting_object" : "setting")
            .arg(setting)
            .arg(widgetMap[setting]->value());
}

void SettingsForm::substituteDependentValues(QDomElement &settingsElement)
{
    if (dependerOnUpdate.isEmpty())
        return; // just safety
    QDomElement levelsElement = XMLAccessor::childElement(settingsElement, "levels");
    if (levelsElement.tagName() != "command")
        return;
//    XMLelement cmd = settingsElement["levels"];
    QDomElement dependerOnUpdateElement = XMLAccessor::childElement(rootElement, dependerOnUpdate);
    QDomElement valueDependerOnUpdateElement = XMLAccessor::childElement(dependerOnUpdateElement, "value");

    QString value = XMLAccessor::value(valueDependerOnUpdateElement);
    QDomElement cmdToken = levelsElement.firstChildElement();
    while (!cmdToken.isNull())
    {
        if (XMLAccessor::label(cmdToken) == QString("$%1").arg(dependerOnUpdate))
        {
            XMLAccessor::setValue(cmdToken, value);
            qDebug() << XMLAccessor::value(cmdToken);
        }

        cmdToken = cmdToken.nextSiblingElement();
    }
}

void SettingsForm::setSetting(QString setting, QString value)
{
    if (!widgetMap.value(setting))
        return;
    bool forceEmitValueChanged = widgetMap[setting]->value() == value && !value.isEmpty() && value != "NULL";
    widgetMap[setting]->setValue(value);
    widgetMap[setting]->setValueSetTrue();
    if (forceEmitValueChanged)
        widgetMap[setting]->emitValueChanged();

}

void SettingsForm::setCurrentTab(QString name)
{
    QWidget *widget = twidgetMap.value(name, nullptr);
    if (widget)
        setCurrentWidget(widget);
}

bool SettingsForm::allIsSet()
{
    bool result = true;
    foreach (PlotSettingsBaseWidget* widget, widgetMap)
        result &= !widget->value().isEmpty();

    return result;
}
