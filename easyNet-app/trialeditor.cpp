#include "trialeditor.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "trialxml.h"
#include "sessionmanager.h"

#include <QDebug>
#include <QLabel>
#include <QToolBar>
#include <QAction>

TrialEditor::TrialEditor(QWidget *parent)
    : QMainWindow(parent), form(nullptr)
{
    trialFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    trialDescriptionUpdater = new ObjectUpdater(this);
    trialDescriptionUpdater->setProxyModel(trialFilter);
    connect(SessionManager::instance(), SIGNAL(currentTrialChanged(QString)), this, SLOT(setTrialName(QString)));
    connect(trialDescriptionUpdater,SIGNAL(objectUpdated(QDomDocument*, QString)),
            this,SLOT(buildForm(QDomDocument*)));

    trialToolBar = new QToolBar;
    QAction* loadTrialAct = new QAction(QIcon(":/images/open.png"), tr("Load trial"), this);
    loadTrialAct->setStatusTip(tr("Load trial"));
    connect(loadTrialAct, SIGNAL(triggered()), this, SIGNAL(loadTrialSignal()));

    trialToolBar->addAction(loadTrialAct);
    addToolBar(trialToolBar);

    scrollArea = new QScrollArea;
    setCentralWidget(scrollArea);
}

TrialEditor::~TrialEditor()
{
}

void TrialEditor::setTrialName(QString name)
{
    trialFilter->setName(name);
    if (name.isEmpty())
        delete scrollArea->takeWidget();
}

void TrialEditor::buildForm(QDomDocument *domDoc)
{
    form = new TrialXML(domDoc->documentElement(), this);
    form->build();
    scrollArea->setWidget(form);
}


