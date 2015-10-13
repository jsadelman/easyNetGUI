#include "trialeditor.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "trialxml.h"
#include "sessionmanager.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>


TrialEditor::TrialEditor(QWidget *parent)
    : QScrollArea(parent)
{
    trialFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    trialDescriptionUpdater = new ObjectUpdater(this);
    trialDescriptionUpdater->setProxyModel(trialFilter);
    connect(trialDescriptionUpdater,SIGNAL (objectUpdated(QDomDocument*)),
            this,SLOT(buildForm(QDomDocument*)));
}

TrialEditor::~TrialEditor()
{
}

void TrialEditor::setTrialName(QString name)
{
    if (!name.isEmpty())
        trialFilter->setName(name);
}

void TrialEditor::buildForm(QDomDocument *domDoc)
{
    form = new TrialXML(domDoc->documentElement());
    form->build();
    setWidget(form);
}

