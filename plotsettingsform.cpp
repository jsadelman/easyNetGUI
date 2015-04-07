#include "plotsettingsform.h"
#include "plotsettingsbasewidget.h"

#include <QDomDocument>
#include <QVBoxLayout>
#include <QMap>
#include <QMetaObject>
#include <QDebug>
#include <QVBoxLayout>


PlotSettingsForm::PlotSettingsForm(QDomDocument *plotSettingsXML, QWidget *parent) :
    QWidget(parent)
{
    widgetList.clear();
    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
    QDomElement settingsElement = plotSettingsXML->firstChildElement().firstChildElement();
    while (!settingsElement.isNull())
    {
        PlotSettingsBaseWidget *widget = createWidget(settingsElement);
        widgetList.append(widget);
        mainLayout->addWidget(widget);
        settingsElement = settingsElement.nextSiblingElement();
    }
    setLayout(mainLayout);
}

QMap<QString, QString> PlotSettingsForm::getSettings()
{
    QMap<QString, QString> settingsMap;
    foreach (PlotSettingsBaseWidget *widget, widgetList) {
        settingsMap[widget->getName()] = widget->getValue();
    }
    return settingsMap;
}

void PlotSettingsForm::setFactorList()
{
    QStringList factorList{"word", "make", "move"};
    foreach(PlotSettingsBaseWidget *widget, widgetList)
    {
        if (QString(widget->metaObject()->className()) == "PlotSettingsFactorWidget")
            qobject_cast<PlotSettingsFactorWidget*>(widget)->createlistEdit(factorList);
    }
}

void PlotSettingsForm::addWidget(QWidget *widget)
{
    mainLayout->insertWidget(mainLayout->indexOf(qobject_cast<QWidget*>(sender()))+1, widget);
}

PlotSettingsBaseWidget *PlotSettingsForm::createWidget(QDomElement settingsElement)
{
    QMap<QString, QString> tagsMap ;
    tagsMap["name"] = settingsElement.attribute("label");
    QDomElement attributeElement = settingsElement.firstChildElement();
    while (!attributeElement.isNull())
    {
        tagsMap[attributeElement.attribute("label")] = attributeElement.attribute("value");
        attributeElement = attributeElement.nextSiblingElement();
    }
    if (tagsMap["type"] == "numeric")
        return new PlotSettingsNumericWidget(tagsMap["name"], tagsMap["value"],
                tagsMap["comment"], tagsMap["default"]);

    else if (tagsMap["type"] == "free") // will be "factor"
    {
        PlotSettingsFactorWidget *widget = new PlotSettingsFactorWidget(tagsMap["name"], tagsMap["value"],
                tagsMap["comment"], tagsMap["default"]);
        connect(widget, SIGNAL(addWidget(QWidget*)), this, SLOT(addWidget(QWidget*)));
        return widget;
    }

    else
        return new PlotSettingsBaseWidget(tagsMap["name"], tagsMap["value"],
            tagsMap["comment"], tagsMap["default"]);
}




