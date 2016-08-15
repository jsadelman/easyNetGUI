#include "objectupdater.h"
#include "objectcache.h"
#include "objectcachefilter.h"
#include "lazynutobject.h"
#include "sessionmanager.h"
#include "lazynutjobparam.h"
#include "lazynutjob.h"
#include "enumclasses.h"
#include "xmlelement.h"

#include <QSortFilterProxyModel>
#include <QDebug>

ObjectUpdater::ObjectUpdater(QObject *parent)
    : m_command(""), m_dependencies(false), QObject(parent), m_awake(false)
{
}

void ObjectUpdater::setProxyModel(QSortFilterProxyModel *proxy)
{
    objectCache = qobject_cast<ObjectCache*>(proxy->sourceModel());
    if (!objectCache)
    {
        eNerror << "proxy model not compatible with ObjectCache";
        return;
    }
    proxyModel = proxy;
    connect(proxyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,const QVector<int> &)),
            this, SLOT(requestObjects(QModelIndex,QModelIndex)), Qt::UniqueConnection);
    connect(proxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(requestObjects(QModelIndex,int,int)), Qt::UniqueConnection);
    wakeUpUpdate();
}

void ObjectUpdater::setFilter(ObjectCacheFilter *filter)
{
    objectCache = qobject_cast<ObjectCache*>(filter->sourceModel());
    if (!objectCache)
    {
        eNerror << "proxy model not compatible with ObjectCache";
        return;
    }
    if (!filter)
    {
        qDebug() << "ERROR: ObjectUpdater::setFilter invalid filter";
        return;
    }
    proxyModel = filter;
    connect(static_cast<ObjectCacheFilter *>(proxyModel), SIGNAL(objectModified(QString)),
            this, SLOT(requestObject(QString)));
    connect(static_cast<ObjectCacheFilter *>(proxyModel), SIGNAL(objectCreated(QString, QString, QString, QDomDocument*)),
            this, SLOT(requestObject(QString)));
    wakeUpUpdate();

}

void ObjectUpdater::wakeUpUpdate()
{
    m_awake = true;
}

void ObjectUpdater::goToSleep()
{
    m_awake = false;
}

void ObjectUpdater::requestObjects(QModelIndex top, QModelIndex bottom)
{
    if (m_awake)
        requestObjects(top.row(), bottom.row());
}

void ObjectUpdater::requestObjects(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)
    if (m_awake)
        requestObjects(first, last);
}

void ObjectUpdater::errorHandler(QString cmd, QString error)
{
    eNerror << cmd << error;
//    QString nameInCmd = cmd.remove(QRegExp("^\\s*xml\\s*|\\s*description\\s*$"));
//    if (error.contains(QString("ERROR: Object %1 does not exist.").arg(nameInCmd)))
//    {
//        objectCache->destroy(nameInCmd);
//    }
}

void ObjectUpdater::requestObjects(int first, int last)
{
    if (m_awake)
    {
        foreach (QString name, getObjectNames(first, last))
        {
            requestObject(name);
        }
    }
}

QStringList ObjectUpdater::getObjectNames(int first, int last)
{
    QStringList names;
    for (int row = first; row <= last; ++row)
    {
        QString name = proxyModel->data(proxyModel->index(row,ObjectCache::NameCol)).toString();
        // this is a workaround
        // QSortFilterProxyModel behaves differently when its QRegExp is all-pass (even if not empty, like ".*".)
        // (probably) signals directly from the source model are used instead of the proxy ones.
        // as a consequence, when creating a new row, the rowsInserted signal from the model is sent
        // by the proxy, but at raw creation the name is still empty.
        // THis does not happen when the QRegExp does actual filetering.
        if (!name.isEmpty())
            names.append(name);
        else
            qDebug() << "WARNING: ObjectUpdater::getObjectNames name not found at row" << row;

    }
    return names;
}

void ObjectUpdater::requestObject(QString name, QString command)
{
    if (!m_awake)
        return;
    if (name.isEmpty())
    {
        qDebug() << "ERROR: ObjectUpdater::requestObject for empty name";
        return;
    }

    if (objectCache->isInvalid(name) && objectCache->isPending(name))
    {
        objectCache->setPending(name, false);
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = QStringList({QString("xml %1 %2").arg(name).arg(command.isEmpty() ? m_command : command)});
        job->setAnswerReceiver(objectCache, SLOT(setDomDocAndValidCache(QDomDocument*, QString)), AnswerFormatterType::XML);
        job->appendErrorReceiver(this, SLOT(errorHandler(QString, QString)));
        SessionManager::instance()->submitJobs(job);
    }
    else if (!objectCache->isInvalid(name) && !objectCache->isPending(name))
    {
        QDomDocument *domDoc = objectCache->getDomDoc(name);
        emit objectUpdated(domDoc, name);
        if (m_dependencies && domDoc)
            foreach (QString dep, XMLelement(*domDoc)["Dependencies"].listValues())
                requestObject(dep);
    }
}
