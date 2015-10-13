#include "descriptionupdater.h"
#include "objectcache.h"
#include "lazynutobject.h"
#include "sessionmanager.h"
#include "lazynutjobparam.h"
#include "lazynutjob.h"
#include "enumclasses.h"
#include <QSortFilterProxyModel>
#include <QDebug>

DescriptionUpdater::DescriptionUpdater(QObject *parent)
    : QObject(parent)
{

}

void DescriptionUpdater::setProxyModel(QSortFilterProxyModel *proxy)
{
    objectCache = qobject_cast<ObjectCache*>(proxy->sourceModel());
    if (!objectCache)
    {
        qDebug() << "ERROR: DescriptionUpdater: proxy model not compatible with ObjectCache";
        return;
    }
    proxyModel = proxy;
    wakeUpUpdate();


}

void DescriptionUpdater::wakeUpUpdate()
{
//    qDebug() << this << "wakeUpUpdate";
    connect(proxyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(requestDescriptions(QModelIndex,QModelIndex)));
    connect(proxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(requestDescriptions(QModelIndex,int,int)));
}

void DescriptionUpdater::goToSleep()
{
//    qDebug() << this << "go to sleep ";
    disconnect(proxyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(requestDescriptions(QModelIndex,QModelIndex)));
    disconnect(proxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(requestDescriptions(QModelIndex,int,int)));
}

void DescriptionUpdater::requestDescriptions(QModelIndex top, QModelIndex bottom)
{
    requestDescriptions(top.row(), bottom.row());
}

void DescriptionUpdater::requestDescriptions(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)
    requestDescriptions(first, last);
}

void DescriptionUpdater::errorHandler(QString cmd, QStringList errorList)
{
    QString nameInCmd = cmd.remove(QRegExp("^\\s*xml\\s*|\\s*description\\s*$"));
    if (errorList.contains(QString("ERROR: Object %1 does not exist.").arg(nameInCmd)))
    {
        objectCache->destroy(nameInCmd);
    }
}

void DescriptionUpdater::requestDescriptions(int first, int last)
{
    foreach (QString name, getObjectNames(first, last))
            requestDescription(name);
}

QStringList DescriptionUpdater::getObjectNames(int first, int last)
{
    QStringList names;
    for (int row = first; row <= last; ++row)
    {
        QString name = proxyModel->data(proxyModel->index(row,0)).toString();
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

void DescriptionUpdater::requestDescription(QString name)
{
    if (name.isEmpty())
        return;
    if (objectCache->isInvalid(name) && objectCache->isPending(name))
    {

        objectCache->setPending(name, false);
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = QStringList({QString("xml %1").arg(name)});
        job->setAnswerReceiver(objectCache, SLOT(setDomDocAndValidCache(QDomDocument*, QString)), AnswerFormatterType::XML);
        job->appendErrorReceiver(this, SLOT(errorHandler(QString, QStringList)));
        SessionManager::instance()->submitJobs(job);
    }
    else if (!objectCache->isInvalid(name) && !objectCache->isPending(name))
    {
        emit descriptionUpdated(objectCache->getDomDoc(name));
    }
}
