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
Arrow::Arrow():m_startItem(0),m_endItem(0)
{
    m_line=new QGraphicsLineItem(this);
    m_head=new QGraphicsPolygonItem(this);
    addToGroup(m_line);
    QPen pen;
    pen.setWidth(3);
    pen.setColor(Qt::black);
    m_line->setPen(pen);
}

void Arrow::setNewEndpoint(Arrow::End end, QPointF pt, Box *bx, Arrow::Strategy)
{
  qDebug()<<"setNewEndpoint";
  if(end==Arrow::SRCPT)
  {
      if(m_startItem!=bx)
      {
          m_startItem->removeArrow(this);
          m_startItem=bx;
          if(m_startItem!=m_endItem) m_arrowType=Arrow::Line; else m_arrowType=Arrow::SelfLoop;
          bx->addArrow(this);
      }
  }
  if(end==Arrow::DSTPT)
  {
      if(m_endItem!=bx)
      {
          m_endItem->removeArrow(this);
          m_endItem=bx;
          if(m_startItem!=m_endItem) m_arrowType=Arrow::Line; else m_arrowType=Arrow::SelfLoop;
          bx->addArrow(this);
      }
  }
}

void Arrow::initWithConnection(Box *from, Box *to)
{
    qDebug()<<"initWith";
  m_startItem=from;
  m_endItem=to;
  if(m_startItem!=m_endItem) m_arrowType=Arrow::Line; else m_arrowType=Arrow::SelfLoop;
  from->addArrow(this);
  to->addArrow(this);
  updatePosition();
}

void Arrow::setDashedStroke(bool)
{

}

void Arrow::updatePosition()
{
//    removeFromGroup(m_line);
    prepareGeometryChange();
    QLineF line;
    line.setP1(m_startItem->connectionPoint(this)+m_startItem->center());
    line.setP2(m_endItem->connectionPoint(this)+m_endItem->center());
    m_line->setLine(line);
//    addToGroup(m_line);
    qDebug()<<line;
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
    if (event->button() == Qt::LeftButton)
    {
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        // Drop through to parent handler.
    }
    else if (event->button() == Qt::RightButton)
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

