#include "objectcachefilter.h"
#include "objectcache.h"

#include <QDebug>
#include <QDomDocument>
Q_DECLARE_METATYPE(QDomDocument*)

ObjectCacheFilter::ObjectCacheFilter(ObjectCache *objectCache, QObject *parent)
    : nameList(), typeList(), QSortFilterProxyModel(parent)
{
    setSourceModel(objectCache);
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(sendObjectCreated(QModelIndex,int,int)));
    connect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sendObjectDestroyed(QModelIndex,int,int)));
    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(sendObjectModified(QModelIndex,QModelIndex,QVector<int>)));
    setNoFilter(); // initialise with a no-pass filter

}

bool ObjectCacheFilter::isAllValid()
{
    bool invalid(false);
    for (int row = 0; row < rowCount(); ++row)
    {
        invalid |= data(index(row,ObjectCache::InvalidCol)).toBool();
    }
    return !invalid;
}

void ObjectCacheFilter::setNoFilter()
{
    setFilterRegExp("^$");
    nameList.clear();
    typeList.clear();
}

void ObjectCacheFilter::setName(QString txt)
{
    nameList = QStringList({txt});
    setList(nameList);
    setFilterKeyColumn(ObjectCache::NameCol);
}

void ObjectCacheFilter::setNameList(QStringList list)
{
    nameList = list;
    setList(nameList);
    setFilterKeyColumn(ObjectCache::NameCol);
}

void ObjectCacheFilter::addName(QString txt)
{
    if (!nameList.contains(txt))
    {
        nameList.append(txt);
        setList(nameList);
    }
    setFilterKeyColumn(ObjectCache::NameCol);
}

void ObjectCacheFilter::removeName(QString txt)
{
    nameList.removeAll(txt);
    setList(nameList);
    setFilterKeyColumn(ObjectCache::NameCol);
}

void ObjectCacheFilter::setType(QString txt)
{
    typeList = QStringList({txt});
    setList(typeList);
    setFilterKeyColumn(ObjectCache::TypeCol);
}

void ObjectCacheFilter::setTypeList(QStringList list)
{
    typeList = list;
    setList(typeList);
    setFilterKeyColumn(ObjectCache::TypeCol);
}

void ObjectCacheFilter::addType(QString txt)
{
    if (!typeList.contains(txt))
    {
        typeList.append(txt);
        setList(typeList);
    }
    setFilterKeyColumn(ObjectCache::TypeCol);
}

void ObjectCacheFilter::removeType(QString txt)
{
    typeList.removeAll(txt);
    setList(typeList);
    setFilterKeyColumn(ObjectCache::TypeCol);
}

void ObjectCacheFilter::sendObjectCreated(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)

    for (int row = first; row <= last; ++row)
    {
        QString name = data(index(row,ObjectCache::NameCol)).toString();
        QString type = data(index(row,ObjectCache::TypeCol)).toString();
        QDomDocument* domDoc = static_cast<ObjectCache*>(sourceModel())->getDomDoc(name);
        emit objectCreated(name, type, domDoc);
    }
}

void ObjectCacheFilter::sendObjectDestroyed(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)
    for (int row = first; row <= last; ++row)
    {
        QString name = data(index(row,ObjectCache::NameCol)).toString();
        if (!name.isEmpty())
            emit objectDestroyed(name);
    }
}

void ObjectCacheFilter::sendObjectModified(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles)
{
    Q_UNUSED(roles)
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row)
    {
        QString name = data(index(row,ObjectCache::NameCol)).toString();
        emit objectModified(name);
    }
}

void ObjectCacheFilter::setList(QStringList list)
{
    // e.g. list = {"a" , "(b c)"}
    // rex = '^(a|\(b c\))$'
    for (int i = 0; i < list.length(); ++i)
        list[i] = QRegExp::escape(list[i]);

    QRegExp rex = QRegExp(QString("^(%1)$").arg(list.join('|')));
    setFilterRegExp(rex);
}
