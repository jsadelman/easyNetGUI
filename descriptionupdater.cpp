#include "descriptionupdater.h"
#include "objectcatalogue.h"
#include <QSortFilterProxyModel>
#include <QDebug>

DescriptionUpdater::DescriptionUpdater(QString updaterName, QObject *parent)
    : updaterName(updaterName), QObject(parent)
{
}

void DescriptionUpdater::setProxyModel(QSortFilterProxyModel *proxy)
{
    proxyModel = proxy;
    objectCatalogue = qobject_cast<ObjectCatalogue*>(proxyModel->sourceModel());
    connect(objectCatalogue, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(requestDescriptions(QModelIndex,QModelIndex)));
}

void DescriptionUpdater::requestDescriptions(QModelIndex top, QModelIndex bottom)
{
    for (int row = top.row(); row <= bottom.row(); ++row)
    {
        if (proxyModel->mapFromSource(objectCatalogue->index(row, top.column())).isValid())
        {
            QString name = objectCatalogue->data(objectCatalogue->index(row, 0)).toString();
            if (objectCatalogue->isInvalid(name) && objectCatalogue->isPending(name))
            {
                objectCatalogue->setPending(name, false);
                requestDescription(name);
            }
        }
    }
}

void DescriptionUpdater::requestDescription(QString name)
{
    qDebug() << updaterName << " requestDescription" << name;
    objectCatalogue->setInvalid(name, false);
}
