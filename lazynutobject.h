#ifndef LAZYNUTOBJECT_H
#define LAZYNUTOBJECT_H

#include <QString>
class QDomDocument;

class LazyNutObject
{
public:
    LazyNutObject(QDomDocument *domDoc);
    LazyNutObject(const LazyNutObject& other);
    LazyNutObject& operator=(LazyNutObject other);
    ~LazyNutObject();
    const QString& name = _name;
    const QString& type = _type;
    const QString& subtype = _subtype;

    QString getValue(const QString& label);

protected:
    QDomDocument *domDoc;

private:
    void initProperties();

    QString _name;
    QString _type;
    QString _subtype;
};

#endif // LAZYNUTOBJECT_H
