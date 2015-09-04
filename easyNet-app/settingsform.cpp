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
        QString tabname =  XMLelement(domElement)["pretty name"]();
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
//    QMap<QString, QString>::const_iterator i = m_defaultSettings.constBegin();
//    while (i != m_defaultSettings.constEnd())
//    {
//        setDefaultModelSetting(i.key(), i.value());
//        ++i;
//    }
}

void SettingsForm::initDependersSet()
{
    QDomElement settingsElement = rootElement.firstChildElement();
    while (!settingsElement.isNull())
    {
        foreach (QString depender, XMLelement(settingsElement)["dependencies"].listValues())
            dependersSet.insert(depender);

        settingsElement = settingsElement.nextSiblingElement();
    }
}



QStringList SettingsForm::getSettingsCmdList()
{
    QStringList cmdList;
    foreach (QString setting, XMLelement(rootElement).listLabels())
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


PlotSettingsBaseWidget *SettingsForm::createWidget(QDomElement domElement)
{
    // TODO: implement this with a factory
    QString type = XMLelement(domElement)["type"]();
    QString choice = XMLelement(domElement)["choice"]();
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
        if (XMLelement(settingsElement)["dependencies"].listValues().contains(dependerOnUpdate))
        {
            if (!newDomDoc)
                substituteDependentValues(settingsElement);
            qDebug() << XMLelement(settingsElement)["levels"]();
            widgetMap[XMLelement(settingsElement).label()]->updateWidget(XMLelement(settingsElement));
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

void SettingsForm::substituteDependentValues(QDomElement domElement)
{
    if (dependerOnUpdate.isEmpty())
        return; // just safety
    XMLelement settingsElement(domElement);
    if (!settingsElement["levels"].isCommand())
        return;
    XMLelement cmd = settingsElement["levels"];
    QString value = XMLelement(rootElement)[dependerOnUpdate]["value"]();
    XMLelement cmdToken = cmd.firstChild();
    while (!cmdToken.isNull())
    {
        if (cmdToken.label() == QString("$%1").arg(dependerOnUpdate))
            cmdToken.setValue(value);

        cmdToken = cmdToken.nextSibling();
    }
}

void SettingsForm::setDefaultModelSetting(QString setting, QString value)
{
    widgetMap[setting]->setValue(value);
    widgetMap[setting]->setValueSetTrue();
}
