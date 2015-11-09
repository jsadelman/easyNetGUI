#include "plotsettingsform.h"
#include "plotsettingsbasewidget.h"
#include "lazynutjob.h"
#include "sessionmanager.h"
#include "xmlaccessor.h"

#include <QDomDocument>
#include <QVBoxLayout>
#include <QMap>
#include <QMetaObject>
#include <QDebug>
#include <QVBoxLayout>
#include <QComboBox>


PlotSettingsForm::PlotSettingsForm(QDomDocument *domDoc, QWidget *parent)
    : domDoc(domDoc), m_useRFormat(true), QTabWidget(parent)
{
    rootElement = domDoc->documentElement();
}

PlotSettingsForm::~PlotSettingsForm()
{
    delete domDoc;
}

void PlotSettingsForm::build()
{
    QDomElement domElement = domDoc->documentElement().firstChildElement();
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
        layoutMap[tabname]->setSizeConstraint(QLayout::SetFixedSize);
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

void PlotSettingsForm::initDependersSet()
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



QStringList PlotSettingsForm::getSettingsCmdList()
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

QMap<QString, QString> PlotSettingsForm::getSettings()
{
    QMap<QString, QString> settings;
    foreach (QString setting, widgetMap.keys())
    {
        settings.insert(setting, widgetMap[setting]->value());
    }
    return settings;
}

QString PlotSettingsForm::value(QString label)
{
    return widgetMap[label]->value();
}


PlotSettingsBaseWidget *PlotSettingsForm::createWidget(QDomElement& domElement)
{
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

void PlotSettingsForm::recordValueChange()
{
    PlotSettingsBaseWidget* widget = qobject_cast<PlotSettingsBaseWidget*>(sender());
    hasChanged[widget->name()] = true;
}

void PlotSettingsForm::checkDependencies()
{
    PlotSettingsBaseWidget* widget = qobject_cast<PlotSettingsBaseWidget*>(sender());
    if (widget && dependersSet.contains(widget->name()))
    {
        dependerOnUpdate = widget->name();
//        qDebug() << "PlotSettingsForm::checkDependencies dependerOnUpdate" <<dependerOnUpdate << m_plotName;
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = getSettingsCmdList();
//        qDebug() << job->cmdList;
        job->cmdList.append(QString("xml %1 list_settings").arg(m_plotName));
        job->setAnswerReceiver(this, SLOT(updateDependees(QDomDocument*)), AnswerFormatterType::XML);
        SessionManager::instance()->submitJobs(job);
    }
}

void PlotSettingsForm::updateDependees(QDomDocument* newDomDoc)
{
    delete domDoc;
    domDoc = newDomDoc;
//    qDebug() << newDomDoc->toString();
    rootElement = domDoc->documentElement();
    if (dependerOnUpdate.isEmpty())
        return; // just safety
    QDomElement settingsElement = rootElement.firstChildElement();
    while (!settingsElement.isNull())
    {
        QDomElement dependenciesElement = XMLAccessor::childElement(settingsElement, "dependencies");
        if ((XMLAccessor::listValues(dependenciesElement)).contains(dependerOnUpdate))
            widgetMap[XMLAccessor::label(settingsElement)]->updateWidget(settingsElement);

        settingsElement = settingsElement.nextSiblingElement();
    }
}

void PlotSettingsForm::updateSize()
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



QString PlotSettingsForm::getSettingCmdLine(QString setting)
{
    QDomElement settingsElement = XMLAccessor::childElement(rootElement, setting);
    QDomElement typeElement = XMLAccessor::childElement(settingsElement, "type");
    return QString("%1 %2 %3 %4")
            .arg(m_plotName)
            .arg(XMLAccessor::value(typeElement) == "dataframe" ? "setting_object" : "setting")
            .arg(setting)
            .arg(widgetMap[setting]->value());
}

void PlotSettingsForm::setDefaultModelSetting(QString setting, QString value)
{
    qDebug() << "PlotSettingsForm::setDefaultModelSetting"  << m_plotName << setting << value;
    widgetMap[setting]->setValue(value);
    widgetMap[setting]->setValueSetTrue();
}
