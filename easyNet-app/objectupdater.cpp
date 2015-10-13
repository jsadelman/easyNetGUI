#include "objectupdater.h"
#include "objectcache.h"
#include "lazynutobject.h"
#include "sessionmanager.h"
#include "lazynutjobparam.h"
#include "lazynutjob.h"
#include "enumclasses.h"
#include <QSortFilterProxyModel>
#include <QDebug>

ObjectUpdater::ObjectUpdater(QObject *parent)
    : m_command(""), QObject(parent)
{

}

void ObjectUpdater::setProxyModel(QSortFilterProxyModel *proxy)
{
    objectCache = qobject_cast<ObjectCache*>(proxy->sourceModel());
    if (!objectCache)
    {
        qDebug() << "ERROR: ObjectUpdater: proxy model not compatible with ObjectCache";
        return;
    }
    proxyModel = proxy;
    wakeUpUpdate();


}

void ObjectUpdater::wakeUpUpdate()
{
//    qDebug() << this << "wakeUpUpdate";
    connect(proxyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(requestObjects(QModelIndex,QModelIndex)));
    connect(proxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(requestObjects(QModelIndex,int,int)));
}

void ObjectUpdater::goToSleep()
{
//    qDebug() << this << "go to sleep ";
    disconnect(proxyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(requestObjects(QModelIndex,QModelIndex)));
    disconnect(proxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(requestObjects(QModelIndex,int,int)));
}

void ObjectUpdater::requestObjects(QModelIndex top, QModelIndex bottom)
{
    requestObjects(top.row(), bottom.row());
}

void ObjectUpdater::requestObjects(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)
    requestObjects(first, last);
}

void ObjectUpdater::errorHandler(QString cmd, QStringList errorList)
{
    QString nameInCmd = cmd.remove(QRegExp("^\\s*xml\\s*|\\s*description\\s*$"));
    if (errorList.contains(QString("ERROR: Object %1 does not exist.").arg(nameInCmd)))
    {
        objectCache->destroy(nameInCmd);
    }
}

void ObjectUpdater::requestObjects(int first, int last)
{
    foreach (QString name, getObjectNames(first, last))
            requestObject(name);
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
    }
    return names;
}

void ObjectUpdater::requestObject(QString name)
{
    if (name.isEmpty())
        return;
    if (objectCache->isInvalid(name) && objectCache->isPending(name))
    {
        objectCache->setPending(name, false);
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = QStringList({QString("xml %1 %2").arg(name).arg(m_command)});
        job->setAnswerReceiver(objectCache, SLOT(setDomDocAndValidCache(QDomDocument*, QString)), AnswerFormatterType::XML);
        job->appendErrorReceiver(this, SLOT(errorHandler(QString, QStringList)));
        SessionManager::instance()->submitJobs(job);
    }
    else if (!objectCache->isInvalid(name) && !objectCache->isPending(name))
    {
        emit objectUpdated(objectCache->getDomDoc(name));
    }
}
