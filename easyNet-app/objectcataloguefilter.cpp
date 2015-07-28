#include "objectcataloguefilter.h"
#include "objectcatalogue.h"

#include <QDebug>
#include <QDomDocument>
Q_DECLARE_METATYPE(QDomDocument*)

ObjectCatalogueFilter::ObjectCatalogueFilter(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(ObjectCatalogue::instance());
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(sendObjectCreated(QModelIndex,int,int)));
    connect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sendObjectDestroyed(QModelIndex,int,int)));
    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(sendObjectModified(QModelIndex,QModelIndex,QVector<int>)));
    setName(""); // initialise with a no-pass filter

}

bool ObjectCatalogueFilter::isAllValid()
{
    bool invalid(false);
    for (int row = 0; row < rowCount(); ++row)
    {
        invalid |= data(index(row,ObjectCatalogue::InvalidCol)).toBool();
    }
    return !invalid;
}

void ObjectCatalogueFilter::setName(QString txt)
{
    setList(QStringList({txt}));
    setFilterKeyColumn(ObjectCatalogue::NameCol);
}

void ObjectCatalogueFilter::setNameList(QStringList list)
{
    setList(list);
    setFilterKeyColumn(ObjectCatalogue::NameCol);
}

void ObjectCatalogueFilter::setType(QString txt)
{
    setList(QStringList({txt}));
    setFilterKeyColumn(ObjectCatalogue::TypeCol);
}

void ObjectCatalogueFilter::setTypeList(QStringList list)
{
    setList(list);
    setFilterKeyColumn(ObjectCatalogue::TypeCol);
}

void ObjectCatalogueFilter::sendObjectCreated(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)

    for (int row = first; row <= last; ++row)
    {
        QString name = data(index(row,ObjectCatalogue::NameCol)).toString();
        QString type = data(index(row,ObjectCatalogue::TypeCol)).toString();
        QDomDocument* domDoc = ObjectCatalogue::instance()->description(name);
        emit objectCreated(name, type, domDoc);
    }
}

void ObjectCatalogueFilter::sendObjectDestroyed(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent)
    for (int row = first; row <= last; ++row)
    {
        QString name = data(index(row,ObjectCatalogue::NameCol)).toString();
        if (!name.isEmpty())
            emit objectDestroyed(name);
    }
}

void ObjectCatalogueFilter::sendObjectModified(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles)
{
    Q_UNUSED(roles)
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row)
    {
        QString name = data(index(row,ObjectCatalogue::NameCol)).toString();
        emit objectModified(name);
    }
}

void ObjectCatalogueFilter::setList(QStringList list)
{
    // e.g. list = {"a" , "(b c)"}
    // rex = '^(a|\(b c\))$'
    QRegExp rex = QRegExp(QString("^(%1)$").arg(list.replaceInStrings(QRegExp("([()])"), "\\\\1").join('|')));
    setFilterRegExp(rex);
}

