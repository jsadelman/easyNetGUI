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
    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
//    setTabPosition(QTabWidget::West);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    XMLelement settingsElement = rootElement.firstChild();
    while (!settingsElement.isNull())
    {
        PlotSettingsBaseWidget *widget = createWidget(settingsElement);
        widgetMap[widget->name()] = widget;
        mainLayout->addWidget(widget);
//        addTab(widget, settingsElement.label());
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



QStringList PlotSettingsForm::getSettingsCmdList()
{
    QStringList cmdList;
    foreach (QString setting, rootElement.listLabels())
        cmdList.append(getSettingCmdLine(setting));

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
    qDebug() << "createWidget " << type << choice;
    PlotSettingsBaseWidget *widget;
    if (type == "numeric")
        widget = new PlotSettingsNumericWidget(settingsElement);

    else if ((type == "dataframe" || type == "factor") && choice == "single")
        widget = new PlotSettingsSingleChoiceWidget(settingsElement);

    else if ((type == "dataframe" || type == "factor") && choice == "multiple")
        widget = new PlotSettingsMultipleChoiceWidget(settingsElement);

    else
        widget = new PlotSettingsBaseWidget(settingsElement);

    connect(widget, SIGNAL(valueChanged()), this, SLOT(checkDependencies()));
    return widget;
}

void PlotSettingsForm::checkDependencies()
{
    PlotSettingsBaseWidget* widget = qobject_cast<PlotSettingsBaseWidget*>(sender());
    if (widget && dependersSet.contains(widget->name()))
    {
//        emit updateRequest();
        dependerOnUpdate = widget->name();
        LazyNutJobParam *param = new LazyNutJobParam;
        param->logMode |= ECHO_INTERPRETER; // debug purpose
        param->cmdList = QStringList({
                                         getSettingCmdLine(dependerOnUpdate),
                                         QString("xml %1 list_settings").arg(plotName)
                                     });
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
        {
            widgetMap[settingsElement.label()]->updateWidget(settingsElement);
//            PlotSettingsBaseWidget *newWidget = createWidget(settingsElement);
//            for (int i = 0; i < newWidget->extraWidgets().count(); ++i)
//            {
//                delete widgetMap[settingsElement.label()]->extraWidgets().at(i);
//                widgetMap[settingsElement.label()]->extraWidgets()[i] = newWidget->extraWidgets().at(i);
//            }
//            delete widgetMap[settingsElement.label()];
//            widgetMap[settingsElement.label()] = newWidget;

//            int layoutIndex = mainLayout->indexOf(widgetMap[settingsElement.label()]);
//            PlotSettingsBaseWidget *oldWidget = mainLayout->takeAt(layoutIndex);
//            PlotSettingsBaseWidget *widget = createWidget(settingsElement);
//            mainLayout->insertWidget(layoutIndex, widget);
//            for(int i = 0; i < oldWidget->extraWidgets().count(); ++i)
//            {
//                QWidget* oldExtraWidget = oldWidget->extraWidgets().at(i);
//                QWidget* extraWidget = widget->extraWidgets().at(i);
//                mainLayout->takeAt(layoutIndex + i + 1);
//                mainLayout->insertWidget(layoutIndex + i + 1, extraWidget);
//                delete oldExtraWidget;
//            }
//            delete oldWidget;
//            widgetMap[widget->name()] = widget;
        }
        settingsElement = settingsElement.nextSibling();
    }
}


QString PlotSettingsForm::getSettingCmdLine(QString setting)
{
    return QString("%1 %2 %3 %4")
            .arg(plotName)
            .arg(rootElement[setting]["type"]() == "dataframe" ? "setting_object" : "setting")
            .arg(setting)
            .arg(widgetMap[setting]->value());
}

