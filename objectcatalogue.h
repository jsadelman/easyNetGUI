#ifndef OBJECTCATALOGUE_H
#define OBJECTCATALOGUE_H



#include <QAbstractTableModel>
class QDomDocument;
class LazyNutObjectCacheElem;
using namespace std;

class ObjectCatalogue : public QAbstractTableModel
{
    Q_OBJECT
public:
    static ObjectCatalogue* instance(); // singleton
    ~ObjectCatalogue();
    int rowCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE;
//    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;

    QDomDocument *description(const QString& name);
    bool isInvalid(const QString& name);
    bool isPending(const QString& name);
    QString type(const QString& name);

public slots:
    bool create(const QString& name, const QString& type);
    bool create(QDomDocument* domDoc);
    bool destroy(QStringList names);
    bool destroy(const QString& name);
    bool setDescription(QDomDocument* domDoc);
    bool setDescriptionAndValidCache(QDomDocument* domDoc);
    bool setInvalid(const QString& name, bool invalid);
    bool setPending(const QString& name, bool pending);
    bool invalidateCache(const QString& name);
    bool invalidateCache(QStringList names);


private:
    ObjectCatalogue(QObject * parent = 0);
    ObjectCatalogue(ObjectCatalogue const&){}
    ObjectCatalogue& operator=(ObjectCatalogue const&){}
    static ObjectCatalogue* objectCatalogue;

    int rowFromName(QString name);
    bool setBit(const QString& name, bool bit, int column);
    bool isBit(const QString& name, int column);
    QList <LazyNutObjectCacheElem*> catalogue;


};

#endif // OBJECTCATALOGUE_H
