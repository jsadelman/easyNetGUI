#include "descriptionupdater.h"
#include "objectcatalogue.h"
#include "lazynutobject.h"
#include <QSortFilterProxyModel>
#include <QDebug>

DescriptionUpdater::DescriptionUpdater(QObject *parent)
    : QObject(parent)
{
}

void DescriptionUpdater::setProxyModel(QSortFilterProxyModel *proxy)
{
    objectCatalogue = qobject_cast<ObjectCatalogue*>(proxy->sourceModel());
    if (!objectCatalogue)
    {
        qDebug() << "ERROR: DescriptionUpdater: proxy model not compatible with ObjectCatalogue";
        return;
    }
    proxyModel = proxy;
    connect(proxyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(requestDescriptions(QModelIndex,QModelIndex)));
    connect(proxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(requestDescriptions(QModelIndex,int,int)));
//    connect(proxyModel, &QSortFilterProxyModel::rowsRemoved, [=](QModelIndex m ,int f ,int l){
//        qDebug() << "removed " << f;
//    });

}

void DescriptionUpdater::requestDescriptions(QModelIndex top, QModelIndex bottom)
{
//    qDebug() << top << bottom;
    requestDescriptions(top.row(), bottom.row());
}

void DescriptionUpdater::requestDescriptions(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)
//    qDebug() << first << last;
    requestDescriptions(first, last);
}

void DescriptionUpdater::notifyDescriptionUpdated(QDomDocument *domDoc)
{
    objectCatalogue->setInvalid(AsLazyNutObject(*domDoc).name(), false);
    emit descriptionUpdated(domDoc);
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
    if (objectCatalogue->isInvalid(name) && objectCatalogue->isPending(name))
    {
        qDebug() << "requestDescription" << name;
        objectCatalogue->setPending(name, false);
        objectCatalogue->setInvalid(name, false);
    }
}
