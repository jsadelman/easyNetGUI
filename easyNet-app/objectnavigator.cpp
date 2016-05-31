#include "objectnavigator.h"
#include "lazynutobjectmodel.h"
#include "sessionmanager.h"
#include "expandtofillbutton.h"
#include "objectcache.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "objecttreeview.h"

#include <QTreeView>
#include <QHeaderView>
#include <QToolBar>
#include <QAction>
#include <QDebug>

ObjectNavigator::ObjectNavigator(QWidget *parent)
    : QMainWindow(parent), currentObject("")
{

    descriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    descriptionUpdater = new ObjectUpdater(this);
    descriptionUpdater->setProxyModel(descriptionFilter);

    objectModel = new LazyNutObjectModel(nullptr, this);
    connect(descriptionFilter, SIGNAL(objectDestroyed(QString)),
            objectModel, SLOT(removeDescription(QString)));
    connect(descriptionUpdater, SIGNAL(objectUpdated(QDomDocument*, QString)),
            objectModel, SLOT(updateDescription(QDomDocument*)));

    objectView = new ObjectTreeView(this);
    objectView->setModel(objectModel);
    connect(objectModel, SIGNAL(objectRequested(QString)), this, SLOT(setObject(QString)));
    setCentralWidget(objectView);


    backwardAct = new QAction(QIcon(":/images/Back.png"), "Back", this);
    backwardAct->setEnabled(false);
    forwardAct = new QAction(QIcon(":/images/Forward.png"), "Forward", this);
    forwardAct->setEnabled(false);
    connect(forwardAct, SIGNAL(triggered()), this, SLOT(forward()));
    connect(backwardAct, SIGNAL(triggered()), this, SLOT(back()));
    navigationToolBar = addToolBar(tr("Navigation"));
    navigationToolBar->addAction(backwardAct);
    navigationToolBar->addAction(forwardAct);

}

void ObjectNavigator::setObject(QString name)
{
    if (!currentObject.isEmpty())
    {
        backwardStack.push(currentObject);
        backwardAct->setEnabled(true);
    }
    forwardStack.clear();
    forwardAct->setEnabled(false);

    SessionManager::instance()->descriptionCache->create(name);
    descriptionFilter->setName(name);
    show();
    raise();
    currentObject = name;
}

void ObjectNavigator::forward()
{
    backwardStack.push(currentObject);
    backwardAct->setEnabled(true);
    currentObject = forwardStack.pop();
    forwardAct->setDisabled(forwardStack.isEmpty());

    descriptionFilter->setName(currentObject);
}

void ObjectNavigator::back()
{
    forwardStack.push(currentObject);
    forwardAct->setEnabled(true);
    currentObject = backwardStack.pop();
    backwardAct->setDisabled(backwardStack.isEmpty());

    descriptionFilter->setName(currentObject);
}
