#ifndef ARROW_H
#define ARROW_H

#include "libdunnartcanvas/dunnart_connector.h"

class Arrow: public dunnart::Connector
{
    Q_OBJECT
    Q_PROPERTY (QString name READ name WRITE setName)
    Q_PROPERTY (QString lazyNutType READ lazyNutType WRITE setLazyNutType)

public:
    Arrow();
    virtual QString name(void) const {return m_name;}
    virtual void setName(const QString& name) {m_name = name;}
    virtual QString lazyNutType(void) const {return m_lazyNutType;}
    virtual void setLazyNutType(const QString& lazyNutType) {m_lazyNutType = lazyNutType;}

protected:
    virtual QAction *buildAndExecContextMenu(
            QGraphicsSceneMouseEvent *event, QMenu& menu);

private:
    void lesion();
    void unlesion();

    QString m_name;
    QString m_lazyNutType;
};

#endif // ARROW_H
