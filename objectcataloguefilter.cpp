#include "objectcataloguefilter.h"
#include "objectcatalogue.h"

#include <QDebug>

ObjectCatalogueFilter::ObjectCatalogueFilter(ObjectCatalogue *objectCatalogue, QObject *parent)
    : objectCatalogue(objectCatalogue), QSortFilterProxyModel(parent)
{
    setSourceModel(objectCatalogue);
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(sendAddedDescription(QModelIndex,int,int)));
    connect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sendRemovedDescription(QModelIndex,int,int)));
}

void ObjectCatalogueFilter::setName(QString txt)
{
    setFilterFixedString(txt);
    setFilterKeyColumn(0);
}

void ObjectCatalogueFilter::setNameList(QStringList list)
{
    setList(list);
    setFilterKeyColumn(0);
}

void ObjectCatalogueFilter::setType(QString txt)
{
    setFilterFixedString(txt);
    setFilterKeyColumn(1);
}

void ObjectCatalogueFilter::setTypeList(QStringList list)
{
    setList(list);
    setFilterKeyColumn(1);
}

void ObjectCatalogueFilter::sendAddedDescription(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)
    for (int row = first; row <= last; ++row)
    {
        QString name = data(index(row,0)).toString();
        qDebug () << "addedDescription" << name;
        emit descriptionAdded(name, objectCatalogue->description(name));
    }
}

void ObjectCatalogueFilter::sendRemovedDescription(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)
    for (int row = first; row <= last; ++row)
    {
        QString name = data(index(row,0)).toString();
        qDebug () << "removedDescription" << name;
        emit descriptionRemoved(name);
    }
}

void ObjectCatalogueFilter::setList(QStringList list)
{
    QRegExp rex = QRegExp(QString("^(%1)$").arg(list.join('|')));
    setFilterRegExp(rex);
}

