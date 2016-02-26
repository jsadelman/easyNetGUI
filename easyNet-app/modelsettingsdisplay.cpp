#include "modelsettingsdisplay.h"
#include "settingsxml.h"
#include "lazynutjob.h"
#include "sessionmanager.h"

ModelSettingsDisplay::ModelSettingsDisplay(QWidget *parent)
    : QScrollArea(parent), m_command(""), m_name("")
{
}

void ModelSettingsDisplay::buildForm(QString name)
{
    m_name = name;
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("xml %1 %2").arg(name).arg(m_command)});
    job->setAnswerReceiver(this, SLOT(buildForm(QDomDocument*)), AnswerFormatterType::XML);
    SessionManager::instance()->submitJobs(job);
}

void ModelSettingsDisplay::buildForm(QDomDocument *domDoc)
{
    form = new SettingsXML(domDoc->documentElement());
    form->setTopLabelValue("Model name", m_name);
    form->build();
    setWidget(form);
}
