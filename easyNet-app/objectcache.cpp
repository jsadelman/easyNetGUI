#include "objectcache.h"
#include "enumclasses.h"
#include "lazynutobjectcacheelem.h"
#include "lazynutobject.h"
#include <QMetaObject>
#include <QDomDocument>
#include <QDebug>

Q_DECLARE_METATYPE(QDomDocument*)

ObjectCache::ObjectCache(QObject *parent)
    : QAbstractTableModel(parent)
{
}


ObjectCache::~ObjectCache()
{
    foreach (LazyNutObjectCacheElem* elem, cache)
        delete elem;
}

int ObjectCache::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return cache.count();
}

int ObjectCache::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return COLUMN_COUNT;
}

QVariant ObjectCache::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= cache.count() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        LazyNutObjectCacheElem* obj = cache.at(index.row());
        if (!obj)
            return QVariant();
        switch(index.column())
        {
        case NameCol:
            return obj->name;
        case TypeCol:
            return obj->type;
        case SubtypeCol:
            return obj->subtype;
        case InvalidCol:
            return obj->invalid;
        case DomDocCol:
            return QVariant::fromValue(obj->domDoc);
        default:
            return QVariant();
        }
    }
    return QVariant();
}

QVariant ObjectCache::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case NameCol:
            return "Name";
        case TypeCol:
            return "Type";
        case SubtypeCol:
            return "Subtype";
        case InvalidCol:
            return "Invalid";
        case DomDocCol:
            return "QDomDocument*";
        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool ObjectCache::setData(const QModelIndex &index, const QVariant &value, int role)
{
     if (index.isValid() && role == Qt::EditRole &&
        index.row() >= 0 && index.row() < cache.count())
     {
         switch(index.column())
         {
         case NameCol:
             cache.at(index.row())->name = value.toString();
             break;
         case TypeCol:
             cache.at(index.row())->type = value.toString();
             break;
         case SubtypeCol:
             cache.at(index.row())->subtype = value.toString();
             break;
         case InvalidCol:
             cache.at(index.row())->invalid = value.toBool();
             break;
         case DomDocCol:
             cache.at(index.row())->domDoc = value.value<QDomDocument*>();
             break;
         default:
             return false;
         }
         emit(dataChanged(index, index));
         return true;
     }
     return false;
}

Qt::ItemFlags ObjectCache::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool ObjectCache::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    Q_ASSERT(count <=1); // since the for loop is wrong, need to be fixed.
    if (row < 0 || row >= cache.count() || row+count > cache.count())
        return false;

    beginRemoveRows(QModelIndex(), row, row+count-1);
    for (int i=row+count-1; i >= row; --i)
    {
        delete cache.at(i);
        cache.removeAt(i);
    }
    endRemoveRows();
    return true;
}

void ObjectCache::clear()
{
    if (rowCount() < 1)
        return;
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    foreach (LazyNutObjectCacheElem* elem, cache)
        delete elem;

    cache.clear();
    endRemoveRows();
}

bool ObjectCache::create(const QString &name, const QString &type, const QString &subtype)
{
    if (rowFromName(name) >= 0) // name exists already
    {
        qDebug()  << "name exists already:" << name;
        return false;
    }

    beginInsertRows(QModelIndex(), 0, 0);
    LazyNutObjectCacheElem *elem = new LazyNutObjectCacheElem(name, type, subtype);
    cache.insert(0, elem);
    endInsertRows();
//    emit dataChanged(index(0,0), index(0,columnCount()-1));
    return true;
}

bool ObjectCache::destroy(const QString &name)
{
    int row = rowFromName(name);
    if (row > -1 && row < rowCount())
        return removeRow(row);
    return false;
}


bool ObjectCache::setDomDocAndValidCache(QDomDocument *domDoc, QString cmd)
{
    QString name = nameFromCmd(cmd);
    if (!exists(name))
    {
        qDebug() << "ERROR: ObjectCache::setDomDocAndValidCache non-existing name" << name;
        return false;
    }

    int row = rowFromName(name);
    if (!setData(index(row, DomDocCol), QVariant::fromValue(domDoc)))
        return false;
    return setInvalid(name, false);
}

bool ObjectCache::invalidateCache(const QString &name)
{
    if (rowFromName(name) == -1)
    {
        // this can happen any time an object is listed both in recently_created and recently_modified
        // then the latter tries to invalidate cache of an object that is not yet in the cache,
        // since the XML has not beed received yet from lazyNut.
//        qDebug() << "WARNING: ObjectCache::invalidateCache no row corresponding to" << name;
        return false;
    }
    // FIRST set pending, THEN invalid, since only the latter triggers signals to filters
    // If reversed the signal would reach a filter still with the pending bit false,
    // thus preventing updaters to request updates.
    if (!setPending(name, true))
    {
        qDebug() << "ERROR: ObjectCache::invalidateCache cannot set pending " << name;
        return false;
    }
    if (!setInvalid(name, true))
    {
        qDebug() << "ERROR: ObjectCache::invalidateCache cannot invalidate " << name;
        return false;
    }
    return true;
}

QDomDocument *ObjectCache::getDomDoc(const QString &name)
{
    if (name.isEmpty() || !exists(name))
    {
//        eNwarning << QString("object %1 does not exist").arg(name);
        return nullptr;
    }
    QVariant v = data(index(rowFromName(name), DomDocCol));
    if (v.canConvert<QDomDocument *>())
        return v.value<QDomDocument *>();
    else
    {
        eNerror << "cannot get QDomDocument for" << name;
        return nullptr;
    }
}

bool ObjectCache::setInvalid(const QString &name, bool invalid)
{
    return setBit(name, invalid, InvalidCol);
}

bool ObjectCache::isInvalid(const QString &name)
{
    return isBit(name, InvalidCol);
}

bool ObjectCache::setPending(const QString &name, bool pending)
{
    int row = rowFromName(name);
    if (row <0)
        return false;

    cache.at(row)->pending = pending;
    return true;
}

bool ObjectCache::isPending(const QString &name)
{
    // should check existence and throw something in case
    return cache.at(rowFromName(name))->pending;
}

QString ObjectCache::type(const QString &name)
{
    return data(index(rowFromName(name), TypeCol)).toString();
}

QString ObjectCache::subtype(const QString &name)
{
    return data(index(rowFromName(name), SubtypeCol)).toString();
}

bool ObjectCache::exists(const QString &name)
{
    QModelIndexList nameMatchList = match(
                index(ObjectCache::NameCol,0),
                Qt::DisplayRole,
                name,
                1,
                Qt::MatchExactly);
    return (nameMatchList.length() > 0);
}

bool ObjectCache::create(QDomDocument *domDoc)
{
    bool success = true;
    XMLelement elem = XMLelement(*domDoc).firstChild();
    while (!elem.isNull())
    {
        success *= create(elem(), elem["type"](), elem["subtype"]());
        elem = elem.nextSibling();
    }
    return success;
}

bool ObjectCache::destroy(QStringList names)
{
    bool success = true;
    foreach (QString name, names)
        success *= destroy(name);

    return success;
}

bool ObjectCache::invalidateCache(QStringList names)
{
    bool success = true;
    foreach (QString name, names)
    {
        success *= invalidateCache(name);
    }
    return success;
}

bool ObjectCache::modify(QDomDocument *domDoc)
{
    bool success = true;
    XMLelement elem = XMLelement(*domDoc).firstChild();
    while (!elem.isNull())
    {
        success *= invalidateCache(elem());
        if (!elem["subtype"]().isEmpty())
            success *= changeSubtype(elem(), elem["subtype"]());
        elem = elem.nextSibling();
    }
    return success;
}

bool ObjectCache::changeSubtype(const QString &name, const QString &subtype)
{
    return setData(index(rowFromName(name), SubtypeCol), subtype);
}

QString ObjectCache::nameFromCmd(QString cmd)
{
    cmd.remove(QRegExp("^\\s*xml\\s+|\\s+description\\s*$|\\s+get.*$"));
    return cmd.simplified();
}

int ObjectCache::rowFromName(const QString& name)
{
    // will include a cache
    QModelIndexList list = match(index(0,NameCol), Qt::DisplayRole, name, 1, Qt::MatchExactly);
    if (!list.isEmpty())
        return list.at(0).row();
    else
        return -1;
}

bool ObjectCache::setBit(const QString &name, bool bit, int column)
{
    if (column == InvalidCol)
        return setData(index(rowFromName(name), column), bit);
    else
        return false;
}

bool ObjectCache::isBit(const QString &name, int column)
{
    QVariant v = data(index(rowFromName(name), column));
    if (!v.isNull() && v.canConvert(QMetaType::Bool))
        return v.toBool();
    else
        return true; // default to invalid
}

