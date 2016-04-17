#include "arrow.h"
#include "sessionmanager.h"
#include "xmlelement.h"
#include "objectcache.h"
#include "box.h"
#include <QDebug>
#include <QMenu>
#include <QApplication>
#include <QGraphicsSceneEvent>
#include <QPen>

Arrow::Arrow():m_startItem(0),m_endItem(0),m_head(0),m_line(0),m_arrowType(Arrow::Unset)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
}

void Arrow::setArrowType(Arrow::ArrowType typ)
{
    prepareGeometryChange();
    if(m_arrowType!=typ)
    {
        if(m_line)
        {
          removeFromGroup(m_line);
          delete m_line;
        }
    if(m_head)
    {
        removeFromGroup(m_head);
        delete m_head;
     }
    switch(typ)
        {
          case(Arrow::Line):
            {
                auto line=new QGraphicsLineItem(this);
                m_line=line;
                auto head=new QGraphicsPolygonItem(this);
                m_head=head;
                QPen pen;
                pen.setWidth(4);
                pen.setColor(Qt::black);
                line->setPen(pen);
                head->setPen(pen);
                head->setBrush(QBrush(Qt::black,Qt::SolidPattern));
                addToGroup(m_line);
                addToGroup(m_head);
                break;
            }
          case(Arrow::SelfLoop):
            {
                auto line=new QGraphicsPathItem(this);
                m_line=line;
                auto head=new QGraphicsEllipseItem(this);
                m_head=head;
                QPen pen;
                pen.setWidth(4);
                pen.setColor(Qt::black);
                line->setPen(pen);
                head->setPen(pen);
                head->setBrush(QBrush(Qt::black,Qt::SolidPattern));
                addToGroup(m_line);
                addToGroup(m_head);
            }
        };

    }
    m_arrowType=typ;

}

void Arrow::setNewEndpoint(Arrow::End end, QPointF pt, Box *bx, Arrow::Strategy)
{
  if(end==Arrow::SRCPT)
  {
      if(m_startItem!=bx)
      {
          m_startItem->removeArrow(this);
          m_startItem=bx;
          if(m_startItem!=m_endItem)
              setArrowType(Arrow::Line);
          else
              setArrowType(Arrow::SelfLoop);
          bx->addArrow(this);
      }
  }
  if(end==Arrow::DSTPT)
  {
      if(m_endItem!=bx)
      {
          m_endItem->removeArrow(this);
          m_endItem=bx;
          if(m_startItem!=m_endItem)
              setArrowType(Arrow::Line);
          else
              setArrowType(Arrow::SelfLoop);
          bx->addArrow(this);
      }
  }
  updatePosition();
}

void Arrow::initWithConnection(Box *from, Box *to)
{
  m_startItem=from;
  m_endItem=to;
  if(m_startItem!=m_endItem)
      setArrowType(Arrow::Line);
  else
      setArrowType(Arrow::SelfLoop);
  from->addArrow(this);
  to->addArrow(this);
  updatePosition();
}

void Arrow::setDashedStroke(bool)
{

}

void Arrow::updatePosition()
{
    prepareGeometryChange();
    QPointF P1(m_startItem->connectionPoint(this)+m_startItem->center());
    QPointF P2(m_endItem->connectionPoint(this)+m_endItem->center());
    switch(m_arrowType)
    {
    case(Arrow::Line):
    {
        auto mline=dynamic_cast<QGraphicsLineItem*>(m_line);

        if(!mline) break;
        QLineF line;
        line.setP1(P1);
        line.setP2(P2);
        mline->setLine(line);
        QGraphicsPolygonItem* mhead=dynamic_cast<QGraphicsPolygonItem*>(m_head);
        if(!mhead) break;
        QPolygonF tri;
        tri.append(P2);
        QLineF temp(P2,P1);
        temp.setLength(20);
        QLineF tempa(temp.p2(),temp.p1());
        QLineF temp2(tempa.normalVector());
        temp2.setLength(10);
        tri.append(temp2.p2());
        temp2.setLength(-10);
        tri.append(temp2.p2());
        mhead->setPolygon(tri);
        break;
    }
    case(Arrow::SelfLoop):
    {
        QGraphicsPathItem*mline=dynamic_cast<QGraphicsPathItem*>(m_line);
        if(!mline)break;
        QPainterPath path=m_startItem->loopPath(this);
        path.translate(m_startItem->center());
        mline->setPath(path);
        QGraphicsEllipseItem*mhead=dynamic_cast<QGraphicsEllipseItem*>(m_head);
        if(!mhead)break;
        QPointF end=path.currentPosition();
        QRectF rect(end.x(),end.y()-5,10,10);
        mhead->setRect(rect);
        break;

    }
    };

}

QAction *Arrow::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu &menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QString subtype;
    QDomDocument *domDoc = SessionManager::instance()->descriptionCache->getDomDoc(m_name);
    if (domDoc)
        subtype =  XMLelement(*domDoc)["subtype"]();


    QAction *lesionAct = menu.addAction(tr("Lesion connection"));
//    lesionAct->setVisible(m_lazyNutType == "connection" && subtype != "lesioned_connection");
    lesionAct->setVisible(m_lazyNutType == "connection" && !dashedStroke());
    QAction *unlesionAct = menu.addAction(tr("Unlesion connection"));
//    unlesionAct->setVisible(m_lazyNutType == "connection" && subtype == "lesioned_connection");
     unlesionAct->setVisible(m_lazyNutType == "connection" && dashedStroke());

     QAction *action = NULL;
     if (!menu.isEmpty())
     {
         QApplication::restoreOverrideCursor();
         action = menu.exec(event->screenPos());
     }


    if (action == lesionAct)
        lesion();

    else if (action == unlesionAct)
        unlesion();

    return action;
}

void Arrow::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
 /*   if (event->button() == Qt::LeftButton)
    {
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        // Drop through to parent handler.
    }
    else*/ if (event->button() == Qt::RightButton)
    {
        QMenu menu;
        QAction *action = buildAndExecContextMenu(event, menu);

        if (action)
        {
            event->accept();
        }
    }

    QGraphicsItem::mousePressEvent(event);
}

void Arrow::lesion()
{
    QString cmd = QString("%1 lesion").arg(m_name);
    SessionManager::instance()->runCmd(cmd);
    // basic version (should check if cmd was executed succesfully)
    setDashedStroke(true);

}


void Arrow::unlesion()
{
    QString cmd = QString("%1 unlesion").arg(m_name);
    SessionManager::instance()->runCmd(cmd);
    // basic version (should check if cmd was executed succesfully)
    setDashedStroke(false);
}

