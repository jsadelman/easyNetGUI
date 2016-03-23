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
    : SettingsForm(domDoc, parent)
{
    m_useRFormat = true;
}


void PlotSettingsForm::recordValueChange(QString oldValue, QString newValue)
{
    Q_UNUSED(oldValue)
    Q_UNUSED(newValue)
    PlotSettingsBaseWidget* widget = qobject_cast<PlotSettingsBaseWidget*>(sender());
    hasChanged[widget->name()] = true;
}

void PlotSettingsForm::triggerUpdateDependees()
{

    LazyNutJob *job = new LazyNutJob;
    job->cmdList = getSettingsCmdList();
    job->cmdList.append(QString("xml %1 list_settings").arg(m_name));
//    qDebug() << Q_FUNC_INFO << m_name <<  job->cmdList << job;
    job->setAnswerReceiver(this, SLOT(updateDependees(QDomDocument*)), AnswerFormatterType::XML);
    SessionManager::instance()->submitJobs(job);
}

//QString PlotSettingsForm::getSettingCmdLine(QString setting)
//{
//    QDomElement settingsElement = XMLAccessor::childElement(rootElement, setting);
//    QDomElement typeElement = XMLAccessor::childElement(settingsElement, "type");
//    return QString("%1 %2 %3 %4")
//            .arg(m_plotName)
//            .arg(XMLAccessor::value(typeElement) == "dataframe" ? "setting_object" : "setting")
//            .arg(setting)
//            .arg(widgetMap[setting]->value());
//}

void PlotSettingsForm::substituteDependentValues(QDomElement &settingsElement)
{
    Q_UNUSED(settingsElement)
    // lazyNut takes care of this
}


