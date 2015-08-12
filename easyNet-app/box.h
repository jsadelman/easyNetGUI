#ifndef BOX_H
#define BOX_H

#include "libdunnartcanvas/shape.h"


class Box: public dunnart::ShapeObj
{
    Q_OBJECT
    Q_PROPERTY (QString name READ name WRITE setName)
    Q_PROPERTY (QString lazyNutType READ lazyNutType WRITE setLazyNutType)

public:
    Box();
    virtual QString name(void) const {return m_name;}
    virtual void setName(const QString& name) {m_name = name;}
    virtual QString lazyNutType(void) const {return m_lazyNutType;}
    virtual void setLazyNutType(const QString& lazyNutType) {m_lazyNutType = lazyNutType;}
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

protected:
    virtual QAction *buildAndExecContextMenu(
            QGraphicsSceneMouseEvent *event, QMenu& menu);

private:
    void enableObserver();
    void disableObserver();

    QString m_name;
    QString m_lazyNutType;
};

#endif // BOX_H
