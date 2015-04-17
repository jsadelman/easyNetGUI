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


PlotSettingsForm::PlotSettingsForm(QDomDocument *domDoc, QString plotName, QWidget *parent)
    : domDoc(domDoc), rootElement(*domDoc), plotName(plotName), QWidget(parent)
{
    widgetList.clear();
    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
    XMLelement settingsElement = rootElement.firstChild();
    while (!settingsElement.isNull())
    {
        PlotSettingsBaseWidget *widget = createWidget(settingsElement);
        widgetMap[widget->getName()] = widget;
        mainLayout->addWidget(widget);
        foreach (QWidget* extraWidget, static_cast<PlotSettingsBaseWidget*>(widget)->extraWidgets())
            mainLayout->addWidget(extraWidget);
        settingsElement = settingsElement.nextSibling();
    }
    setLayout(mainLayout);
    initDependersSet();
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
    qDebug() << dependersSet;
}


//QMap<QString, QString> PlotSettingsForm::getSettings()
//{
//    QMap<QString, QString> settingsMap;
//    foreach (PlotSettingsBaseWidget *widget, widgetList) {
//        settingsMap[widget->getName()] = widget->getValue();
//    }
//    return settingsMap;
//}

QStringList PlotSettingsForm::getSettingsCmdList()
{
    QStringList cmdList;
    foreach (QString setting, rootElement.listLabels())
        cmdList.append(getSettingCmdLine(setting));

    return cmdList;
}

//void PlotSettingsForm::setFactorList(QStringList list)
//{
//    foreach(PlotSettingsBaseWidget *widget, widgetList)
//    {
//        if (QString(widget->metaObject()->className()) == "PlotSettingsFactorWidget")
//            qobject_cast<PlotSettingsFactorWidget*>(widget)->createListEdit(list);
//    }
//}

//void PlotSettingsForm::addWidget(QWidget *widget)
//{
//    qDebug () << "addWidget, called by " << qobject_cast<PlotSettingsBaseWidget*>(sender())->getName();
//    qDebug () << "addWidget, main layout count: " << mainLayout->count();
//    mainLayout->insertWidget(mainLayout->indexOf(qobject_cast<QWidget*>(sender()))+1, widget);
//}

//PlotSettingsBaseWidget *PlotSettingsForm::createWidget(QDomElement settingsElement)
//{
//    QMap<QString, QString> tagsMap ;
//    tagsMap["name"] = settingsElement.attribute("label");
//    QDomElement attributeElement = settingsElement.firstChildElement();
//    while (!attributeElement.isNull())
//    {
//        tagsMap[attributeElement.attribute("label")] = attributeElement.attribute("value");
//        attributeElement = attributeElement.nextSiblingElement();
//    }
//    if (tagsMap["type"] == "numeric")
//        return new PlotSettingsNumericWidget(tagsMap["name"], tagsMap["value"],
//                tagsMap["comment"], tagsMap["default"]);

//    else if (tagsMap["type"] == "factor")
//    {
//        PlotSettingsFactorWidget *widget = new PlotSettingsFactorWidget(tagsMap["name"], tagsMap["value"],
//                tagsMap["comment"], tagsMap["default"]);
//        connect(widget, SIGNAL(addWidget(QWidget*)), this, SLOT(addWidget(QWidget*)));
//        return widget;
//    }

//    else
//        return new PlotSettingsBaseWidget(tagsMap["name"], tagsMap["value"],
//                tagsMap["comment"], tagsMap["default"]);
//}

PlotSettingsBaseWidget *PlotSettingsForm::createWidget(XMLelement settingsElement)
{
    // TODO: implement this with a factory
    QString type = settingsElement["type"]();
    PlotSettingsBaseWidget *widget;
    if (type == "numeric")
        widget = new PlotSettingsNumericWidget(settingsElement);

    else if (type == "dataframe")
        widget = new PlotSettingsDataframeWidget(settingsElement);

    else if (type == "factor")
        widget = new PlotSettingsFactorWidget(settingsElement);

    else
        widget = new PlotSettingsBaseWidget(settingsElement);

    connect(widget, SIGNAL(valueChanged()), this, SLOT(checkDependencies()));
    return widget;
}

void PlotSettingsForm::checkDependencies()
{
    PlotSettingsBaseWidget* widget = qobject_cast<PlotSettingsBaseWidget*>(sender());
    if (widget && dependersSet.contains(widget->getName()))
    {
        qDebug() << "updateRequest()";
        emit updateRequest();
    }

}

QString PlotSettingsForm::getSettingCmdLine(QString setting)
{
    return QString("%1 %2 %3 %4")
            .arg(plotName)
            .arg(rootElement[setting]["type"]() == "dataframe" ? "setting_object" : "setting")
            .arg(setting)
            .arg(widgetMap[setting]->getValue());
}

