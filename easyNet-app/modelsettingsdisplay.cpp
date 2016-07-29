#include "modelsettingsdisplay.h"
#include "settingsxml.h"
#include "lazynutjob.h"
#include "sessionmanager.h"
#include "objectcachefilter.h"


ModelSettingsDisplay::ModelSettingsDisplay(QWidget *parent)
    : QScrollArea(parent), m_command(""), m_name("")
{
    modelFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    connect(SessionManager::instance(), SIGNAL(currentModelChanged(QString)), modelFilter, SLOT(setName(QString)));
    connect(modelFilter, &ObjectCacheFilter::objectDestroyed, [=](QString name)
    {
        if (name == m_name)
        {
            delete takeWidget();
            m_name.clear();
        }
    });
//    connect(modelFilter, SIGNAL(objectCreated(QString,QString,QString,QDomDocument*)), this, SLOT(buildForm(QString)));
    connect(modelFilter, SIGNAL(objectModified(QString)), this, SLOT(buildForm(QString)));
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

