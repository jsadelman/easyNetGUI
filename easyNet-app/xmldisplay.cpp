#include "xmldisplay.h"

#include "objectcachefilter.h"
#include "objectupdater.h"
#include "sessionmanager.h"
#include "xmlform.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QDomDocument>


XMLDisplay::XMLDisplay(QWidget *parent)
    : QScrollArea(parent)
{
    objectFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    objectDescriptionUpdater = new ObjectUpdater(this);
    objectDescriptionUpdater->setProxyModel(objectFilter);
    connect(objectDescriptionUpdater,SIGNAL (objectUpdated(QDomDocument*, QString)),
            this,SLOT(buildForm(QDomDocument*)));
}

XMLDisplay::~XMLDisplay()
{
}


void XMLDisplay::setName(QString name)
{
    if (!name.isEmpty())
        objectFilter->setName(name);
}

void XMLDisplay::buildForm(QDomDocument *domDoc)
{
    form = new XMLForm(domDoc->documentElement());
//    newForm(domDoc);
    form->build();
    setWidget(form);
}

void XMLDisplay::newForm(QDomDocument *domDoc)
{
    form = new XMLForm(domDoc->documentElement());
}
