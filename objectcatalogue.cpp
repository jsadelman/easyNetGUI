#include "objectcatalogue.h"
#include "lazynutobjectcacheelem.h"
#include "lazynutobject.h"
#include <QMetaObject>
#include <QDomDocument>
#include <QDebug>

Q_DECLARE_METATYPE(QDomDocument*)

ObjectCatalogue::ObjectCatalogue(QObject *parent)
    : QAbstractTableModel(parent)
{
}

ObjectCatalogue::~ObjectCatalogue()
{
    foreach (LazyNutObjectCacheElem* elem, catalogue)
        delete elem;
}

int ObjectCatalogue::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return catalogue.count();
}

int ObjectCatalogue::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant ObjectCatalogue::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= catalogue.count() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        LazyNutObjectCacheElem* obj = catalogue.at(index.row());
        switch(index.column())
        {
        case 0:
            return obj->name;
        case 1:
            return obj->type;
        case 2:
            return obj->invalid;
        case 3:
            return obj->pending;
        case 4:
//            return obj->domDoc == nullptr ? "" : QString().sprintf("%08p", obj->domDoc);
            return QVariant::fromValue(obj->domDoc);
        default:
            return QVariant();
        }
    }
    return QVariant();
}

QVariant ObjectCatalogue::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return "Name";
        case 1:
            return "Type";
        case 2:
            return "Invalid";
        case 3:
            return "Pending";
        case 4:
            return "QDomDocument*";
        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool ObjectCatalogue::setData(const QModelIndex &index, const QVariant &value, int role)
{
     if (index.isValid() && role == Qt::EditRole &&
        index.row() >= 0 && index.row() < catalogue.count())
     {
         switch(index.column())
         {
         case 0:
             catalogue.at(index.row())->name = value.toString();
             break;
         case 1:
             catalogue.at(index.row())->type = value.toString();
             break;
         case 2:
             catalogue.at(index.row())->invalid = value.toBool();
             break;
         case 3:
             catalogue.at(index.row())->pending = value.toBool();
             break;
         case 4:
             catalogue.at(index.row())->domDoc = value.value<QDomDocument*>();
             break;
         default:
             return false;
         }
         emit(dataChanged(index, index));
         return true;
     }
     return false;
}

Qt::ItemFlags ObjectCatalogue::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

//bool ObjectCatalogue::insertRows(int row, int count, const QModelIndex &parent)
//{
//    Q_UNUSED(parent);
//    if (row < 0 || row > catalogue.count())
//        return false;

//    beginInsertRows(QModelIndex(), row, row+count-1);

//    for (int i=0; i < count; ++i)
//        catalogue.insert(row, new LazyNutObjectCacheElem);

//    endInsertRows();
//    return true;
//}

bool ObjectCatalogue::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (row < 0 || row >= catalogue.count())
        return false;

    beginRemoveRows(QModelIndex(), row, row+count-1);

    for (int i=0; i < count; ++i)
    {
        delete catalogue.at(row);
        catalogue.removeAt(row);
    }

    endRemoveRows();
    return true;
}

bool ObjectCatalogue::create(const QString &name, const QString &type)
{
    if (rowFromName(name) >= 0) // name exists already
        return false;

    beginInsertRows(QModelIndex(), 0, 0);
    LazyNutObjectCacheElem *elem = new LazyNutObjectCacheElem(name, type);
    catalogue.insert(0, elem);
    endInsertRows();
    return true;

}

bool ObjectCatalogue::destroy(const QString &name)
{
    return removeRow(rowFromName(name));
}

bool ObjectCatalogue::setDescription(QDomDocument *domDoc)
{
    QString name = AsLazyNutObject(*domDoc).name();
    int row = rowFromName(name);
    return setData(index(row, 4), QVariant::fromValue(domDoc));
}

bool ObjectCatalogue::setDescriptionAndValidCache(QDomDocument *domDoc)
{
    QString name = AsLazyNutObject(*domDoc).name();

//    if (!setPending(name, false))
//        return false;
    if  (!setDescription(domDoc))
        return false;
    return setInvalid(name, false);
}

bool ObjectCatalogue::invalidateCache(const QString &name)
{
    if (!setInvalid(name, true))
        return false;
    return setPending(name, true);
}

QDomDocument *ObjectCatalogue::description(const QString &name)
{
    QVariant v = data(index(rowFromName(name), 4));
    if (v.canConvert<QDomDocument *>())
        return v.value<QDomDocument *>();
    else
        return nullptr;
}

bool ObjectCatalogue::setInvalid(const QString &name, bool invalid)
{
    return setBit(name, invalid, 2);
}

bool ObjectCatalogue::isInvalid(const QString &name)
{
    return isBit(name, 2);
}

bool ObjectCatalogue::setPending(const QString &name, bool pending)
{
    return setBit(name, pending, 3);
}

bool ObjectCatalogue::isPending(const QString &name)
{
    return isBit(name, 3);
}

QString ObjectCatalogue::type(const QString &name)
{
    return data(index(rowFromName(name), 1)).toString();
}

bool ObjectCatalogue::create(QDomDocument *domDoc)
{
    bool success = true;
    XMLelement elem = XMLelement(*domDoc).firstChild();
    while (!elem.isNull())
    {
        success *= create(elem(), elem["type"]());
        elem = elem.nextSibling();
    }
    return success;
}

bool ObjectCatalogue::destroy(QStringList names)
{
    bool success = true;
    foreach (QString name, names)
        success *= destroy(name);

    return success;
}

bool ObjectCatalogue::invalidateCache(QStringList names)
{
    bool success = true;
    foreach (QString name, names)
        success *= invalidateCache(name);

    return success;
}

int ObjectCatalogue::rowFromName(QString name)
{
    // will include a cache
    QModelIndexList list = match(index(0,0), Qt::DisplayRole, name, 1, Qt::MatchExactly);
    if (!list.isEmpty())
        return list.at(0).row();
    else
        return -1;
}

bool ObjectCatalogue::setBit(const QString &name, bool bit, int column)
{
    if (column == 2 || column == 3)
        return setData(index(rowFromName(name), column), bit);
    else
        return false;
}

bool ObjectCatalogue::isBit(const QString &name, int column)
{
    QVariant v = data(index(rowFromName(name), column));
    if (!v.isNull() && v.canConvert(QMetaType::Bool))
        return v.toBool();
    else
        return true; // default to invalid and pending
}

