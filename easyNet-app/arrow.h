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
    Box* getStartItem()const{return m_startItem;}
    Box* getEndItem()const{return m_endItem;}
    enum ArrowType {Line,SelfLoop};
    enum End {SRCPT,DSTPT};
    enum Strategy {CENTRE_CONNECTION_PIN};
    ArrowType getArrowType()const{return m_arrowType;}
    void setNewEndpoint(End,QPointF,Box*,Strategy);
protected:
    virtual QAction *buildAndExecContextMenu(
            QGraphicsSceneMouseEvent *event, QMenu& menu);

private:
    void lesion();
    void unlesion();

    QString m_name;
    QString m_lazyNutType;
    Box* m_startItem,*m_endItem;
    ArrowType m_arrowType;
};

#endif // ARROW_H
