#include "trialeditor.h"
#include "objectcataloguefilter.h"
#include "descriptionupdater.h"
#include "trialxml.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>


TrialEditor::TrialEditor(QWidget *parent)
    : QScrollArea(parent)
{
    trialFilter = new ObjectCatalogueFilter(this);
    trialDescriptionUpdater = new DescriptionUpdater(this);
    trialDescriptionUpdater->setProxyModel(trialFilter);
    connect(trialDescriptionUpdater,SIGNAL (descriptionUpdated(QDomDocument*)),
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

