#ifndef ARROW_H
#define ARROW_H

#include <QGraphicsItem>
#include <QAction>

#include "diagramitem.h"

class Box;

class Arrow: public DiagramItem
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
    Box* getStartItem()const{return startItem;}
    Box* getEndItem()const{return endItem;}
protected:
    virtual QAction *buildAndExecContextMenu(
            QGraphicsSceneMouseEvent *event, QMenu& menu);

private:
    void lesion();
    void unlesion();

    QString m_name;
    QString m_lazyNutType;
    Box* startItem,*endItem;
};

#endif // ARROW_H
