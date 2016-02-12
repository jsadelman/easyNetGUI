#ifndef OBJECTCACHE_H
#define OBJECTCACHE_H

#include <QAbstractTableModel>
class QDomDocument;
class LazyNutObjectCacheElem;
using namespace std;

class ObjectCache : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum {NameCol = 0, TypeCol, SubtypeCol, InvalidCol, DomDocCol, COLUMN_COUNT};
    explicit ObjectCache(QObject * parent = 0);
    ~ObjectCache();
    int rowCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE;
//    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
    void clear();

    QDomDocument *getDomDoc(const QString& name);
    bool isInvalid(const QString& name);
    bool isPending(const QString& name);
    QString type(const QString& name);
    QString subtype(const QString& name);
    bool exists(const QString& name);

public slots:
    bool create(const QString& name, const QString& type, const QString& subtype=QString());
    bool create(QDomDocument* domDoc);
    bool destroy(QStringList names);
    bool destroy(const QString& name);
    bool setDomDocAndValidCache(QDomDocument* domDoc, QString cmd);
    bool setInvalid(const QString& name, bool invalid);
    bool setPending(const QString& name, bool pending);
    bool invalidateCache(const QString& name);
    bool invalidateCache(QStringList names);


private:

    virtual QString nameFromCmd(QString cmd);
    int rowFromName(const QString &name);
    bool setBit(const QString& name, bool bit, int column);
    bool isBit(const QString& name, int column);
    QList <LazyNutObjectCacheElem*> cache;


};


#endif // OBJECTCACHE_H
