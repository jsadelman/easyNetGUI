#include "objectcatalogue.h"
#include "lazynutobjectcacheelem.h"
#include "lazynutobject.h"
#include <QMetaObject>
#include <QDomDocument>
#include <QDebug>

Q_DECLARE_METATYPE(QDomDocument*)

ObjectCatalogue* ObjectCatalogue::objectCatalogue = nullptr;


ObjectCatalogue::ObjectCatalogue(QObject *parent)
    : QAbstractTableModel(parent)
{
}

ObjectCatalogue *ObjectCatalogue::instance()
{
    return objectCatalogue ? objectCatalogue : (objectCatalogue = new ObjectCatalogue);
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
    return COLUMN_COUNT;
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
        if (!obj)
            return QVariant();
        switch(index.column())
        {
        case NameCol:
            return obj->name;
        case TypeCol:
            return obj->type;
        case InvalidCol:
            return obj->invalid;
        case DescriptionCol:
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
        case NameCol:
            return "Name";
        case TypeCol:
            return "Type";
        case InvalidCol:
            return "Invalid";
        case DescriptionCol:
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
         qDebug() << "setData row col val " << index.row() << index.column() << value;
         switch(index.column())
         {
         case NameCol:
             catalogue.at(index.row())->name = value.toString();
             break;
         case TypeCol:
             catalogue.at(index.row())->type = value.toString();
             break;
         case InvalidCol:
             catalogue.at(index.row())->invalid = value.toBool();
             break;
         case DescriptionCol:
             catalogue.at(index.row())->domDoc = value.value<QDomDocument*>();
             break;
         default:
             return false;
         }
         emit(dataChanged(index, index));
         qDebug () << "set Data emitted data changed row col val"<< index.row() << index.column() << value;

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
    emit dataChanged(index(0,0), index(0,columnCount()-1));
    qDebug () << "create emitted data changed  name"  << name;

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
    return setData(index(row, DescriptionCol), QVariant::fromValue(domDoc));
}

bool ObjectCatalogue::setDescriptionAndValidCache(QDomDocument *domDoc)
{
    QString name = AsLazyNutObject(*domDoc).name();

//    if (!setPending(name, false))
//        return false;
    qDebug () << "before setDescription";

    if  (!setDescription(domDoc))
        return false;
    qDebug () << "after setDescription";
    return setInvalid(name, false);
}

bool ObjectCatalogue::invalidateCache(const QString &name)
{
    if (rowFromName(name) == -1)
        return false;
    if (!setInvalid(name, true))
        return false;
    return setPending(name, true);
}

QDomDocument *ObjectCatalogue::description(const QString &name)
{
    QVariant v = data(index(rowFromName(name), DescriptionCol));
    if (v.canConvert<QDomDocument *>())
        return v.value<QDomDocument *>();
    else
        return nullptr;
}

bool ObjectCatalogue::setInvalid(const QString &name, bool invalid)
{
    return setBit(name, invalid, InvalidCol);
}

bool ObjectCatalogue::isInvalid(const QString &name)
{
    return isBit(name, InvalidCol);
}

bool ObjectCatalogue::setPending(const QString &name, bool pending)
{
    qDebug() << "setPending" << name;
    int row = rowFromName(name);
    if (row <0)
        return false;

    catalogue.at(row)->pending = pending;
    return true;
}

bool ObjectCatalogue::isPending(const QString &name)
{
    // should check existence and throw something in case
    return catalogue.at(rowFromName(name))->pending;
}

QString ObjectCatalogue::type(const QString &name)
{
    return data(index(rowFromName(name), TypeCol)).toString();
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

int ObjectCatalogue::rowFromName(const QString& name)
{
    // will include a cache
    QModelIndexList list = match(index(0,NameCol), Qt::DisplayRole, name, 1, Qt::MatchExactly);
    if (!list.isEmpty())
        return list.at(0).row();
    else
        return -1;
}

bool ObjectCatalogue::setBit(const QString &name, bool bit, int column)
{
    if (column == InvalidCol)
    {
        qDebug () << "setBit " << name;
        return setData(index(rowFromName(name), column), bit);
    }
    else
        return false;
}

bool ObjectCatalogue::isBit(const QString &name, int column)
{
    QVariant v = data(index(rowFromName(name), column));
    if (!v.isNull() && v.canConvert(QMetaType::Bool))
        return v.toBool();
    else
        return true; // default to invalid
}

