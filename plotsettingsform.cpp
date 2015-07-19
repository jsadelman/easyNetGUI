#include "plotsettingsform.h"
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


PlotSettingsForm::PlotSettingsForm(QDomDocument *domDoc, QString plotName,
                                   QMap<QString, QString> defaultSettings, QWidget *parent)
    : domDoc(domDoc), rootElement(*domDoc), plotName(plotName), QTabWidget(parent)
{
//    mainLayout = new QVBoxLayout;
//    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
//    setTabPosition(QTabWidget::West);
//    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    XMLelement settingsElement = rootElement.firstChild();
    while (!settingsElement.isNull())
    {
        PlotSettingsBaseWidget *widget = createWidget(settingsElement);
        widgetMap.insert(widget->name(), widget);
        hasChanged.insert(widget->name(), false);
        QString tabname = settingsElement["pretty name"]();
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
        settingsElement = settingsElement.nextSibling();
    }

    for(auto tabname: tabOrder)
    {
        layoutMap[tabname]->addStretch();
        twidgetMap[tabname]->setLayout(layoutMap[tabname]);
        addTab(twidgetMap[tabname],tabname);
    }
//    setLayout(mainLayout);
    initDependersSet();
//    updateSize();
    QMap<QString, QString>::const_iterator i = defaultSettings.constBegin();
    qDebug() << "defaultSettings = " << defaultSettings;
    while (i != defaultSettings.constEnd())
    {
        setDefaultModelSetting(i.key(), i.value());
        ++i;
    }

}

PlotSettingsForm::~PlotSettingsForm()
{
    delete domDoc;
}

void PlotSettingsForm::initDependersSet()
{
    XMLelement settingsElement = rootElement.firstChild();
    while (!settingsElement.isNull())
    {
        foreach (QString depender, settingsElement["dependencies"].listValues())
            dependersSet.insert(depender);

        settingsElement = settingsElement.nextSibling();
    }
}



QStringList PlotSettingsForm::getSettingsCmdList()
{
    QStringList cmdList;
    foreach (QString setting, rootElement.listLabels())
        if (hasChanged[setting])
        {
            cmdList.append(getSettingCmdLine(setting));
            hasChanged[setting] = false;
        }
    return cmdList;
}

QString PlotSettingsForm::value(QString label)
{
    return widgetMap[label]->value();
}


PlotSettingsBaseWidget *PlotSettingsForm::createWidget(XMLelement settingsElement)
{
    // TODO: implement this with a factory
    QString type = settingsElement["type"]();
    QString choice = settingsElement["choice"]();
    PlotSettingsBaseWidget *widget;
    if (type == "numeric")
        widget = new PlotSettingsNumericWidget(settingsElement);

    else if ((type == "dataframe" || type == "factor") && choice == "single")
        widget = new PlotSettingsSingleChoiceWidget(settingsElement);

    else if ((type == "dataframe" || type == "factor") && choice == "multiple")
        widget = new PlotSettingsMultipleChoiceWidget(settingsElement);

    else
        widget = new PlotSettingsBaseWidget(settingsElement);


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
        LazyNutJobParam *param = new LazyNutJobParam;
        param->logMode |= ECHO_INTERPRETER; // debug purpose
        param->cmdList = getSettingsCmdList();
        param->cmdList.append(QString("xml %1 list_settings").arg(plotName));
        param->answerFormatterType = AnswerFormatterType::XML;
        param->setAnswerReceiver(this, SLOT(updateDependees(QDomDocument*)));
        SessionManager::instance()->setupJob(param, sender());
    }
}

void PlotSettingsForm::updateDependees(QDomDocument* newDomDoc)
{
    delete domDoc;
    domDoc = newDomDoc;
    rootElement = XMLelement(*domDoc);
    if (dependerOnUpdate.isEmpty())
        return; // just safety
    XMLelement settingsElement = rootElement.firstChild();
    while (!settingsElement.isNull())
    {
        if (settingsElement["dependencies"].listValues().contains(dependerOnUpdate))
            widgetMap[settingsElement.label()]->updateWidget(settingsElement);

        settingsElement = settingsElement.nextSibling();
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
    return QString("%1 %2 %3 %4")
            .arg(plotName)
            .arg(rootElement[setting]["type"]() == "dataframe" ? "setting_object" : "setting")
            .arg(setting)
            .arg(widgetMap[setting]->value());
}

void PlotSettingsForm::setDefaultModelSetting(QString setting, QString value)
{
    widgetMap[setting]->setValue(value);
    widgetMap[setting]->setValueSetTrue();
    qDebug() << "Set" << widgetMap[setting]->name() << "to" << widgetMap[setting]->value();
    emit widgetMap[setting]->valueChanged();
//    updateDependees();
}
