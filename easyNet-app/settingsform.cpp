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
   : domDoc(domDoc), QTabWidget(parent)
{
    rootElement = domDoc->documentElement();
//    m_defaultSettings.clear();
}

SettingsForm::~SettingsForm()
{
    delete domDoc;
}


void SettingsForm::build()
{
    QDomElement domElement = rootElement.firstChildElement();
//    XMLelement settingsElement = rootElement.firstChild();
    while (!domElement.isNull())
    {
        PlotSettingsBaseWidget *widget = createWidget(domElement);
        widgetMap.insert(widget->name(), widget);
        hasChanged.insert(widget->name(), false);
        QDomElement prettyElement = XMLAccessor::childElement(domElement, "pretty name");
        QString tabname = XMLAccessor::value(prettyElement);
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
        twidgetMap[tabname]->setLayout(layoutMap[tabname]);
        addTab(twidgetMap[tabname],tabname);
    }
    initDependersSet();
    QMap<QString, QString>::const_iterator i = m_defaultSettings.constBegin();
    while (i != m_defaultSettings.constEnd())
    {
        setDefaultModelSetting(i.key(), i.value());
        ++i;
    }
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



QStringList SettingsForm::getSettingsCmdList()
{
    QStringList cmdList;
    foreach (QString setting, XMLAccessor::listLabels(rootElement))
        if (hasChanged[setting])
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
    QString type = XMLAccessor::value(typeElement);
    QString choice = XMLAccessor::value(choiceElement);
    PlotSettingsBaseWidget *widget;
    if (type == "numeric")
        widget = new PlotSettingsNumericWidget(domElement, m_useRFormat);

    else if ((type == "dataframe" || type == "factor") && choice == "single")
        widget = new PlotSettingsSingleChoiceWidget(domElement, m_useRFormat);

    else if ((type == "dataframe" || type == "factor") && choice == "multiple")
        widget = new PlotSettingsMultipleChoiceWidget(domElement, m_useRFormat);

    else
        widget = new PlotSettingsBaseWidget(domElement, m_useRFormat);


    connect(widget, SIGNAL(valueChanged()), this, SLOT(recordValueChange()));
    connect(widget, SIGNAL(valueChanged()), this, SLOT(checkDependencies()));
//    connect(widget, SIGNAL(sizeChanged()), this, SLOT(updateSize()));
    return widget;
}

void SettingsForm::recordValueChange()
{
    PlotSettingsBaseWidget* widget = qobject_cast<PlotSettingsBaseWidget*>(sender());
    hasChanged[widget->name()] = true;
}

void SettingsForm::checkDependencies()
{
    PlotSettingsBaseWidget* widget = qobject_cast<PlotSettingsBaseWidget*>(sender());
    if (widget && dependersSet.contains(widget->name()))
    {
        dependerOnUpdate = widget->name();
        updateDependees();
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
//    XMLelement settingsElement = rootElement.firstChild();
    QDomElement settingsElement = rootElement.firstChildElement();
    while (!settingsElement.isNull())
    {
        QDomElement dependenciesElement = XMLAccessor::childElement(settingsElement, "dependencies");
        if ((XMLAccessor::listValues(dependenciesElement)).contains(dependerOnUpdate))
        {
            if (!newDomDoc)
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



QString SettingsForm::getSettingCmdLine(QString setting)
{
    // base implementation returns "setting value"
    return QString("%1 %2")
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

void SettingsForm::setDefaultModelSetting(QString setting, QString value)
{
    widgetMap[setting]->setValue(value);
    widgetMap[setting]->setValueSetTrue();
}
