/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "diagramitem.h"
#include "arrow.h"
#include "box.h"
#include <math.h>

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QPair>
#include <QDebug>
#include <QStyleOptionGraphicsItem>
#include <QStyle>
#include <QApplication>

const qreal Pi = 3.14;


//! [0]
DiagramItem::DiagramItem(DiagramType diagramType, QString name, QMenu *contextMenu,
             QGraphicsItem *parent)
    : QGraphicsPolygonItem(parent)
{
    myName = name;
    label = myName;
    myDiagramType = diagramType;
    myContextMenu = contextMenu;
    myColor = Qt::black;
    penWidth = 4;
    setGeometry();


/*    QPainterPath path;
    switch (myDiagramType) {
        case StartEnd:
            path.moveTo(200, 50);
            path.arcTo(150, 0, 50, 50, 0, 90);
            path.arcTo(50, 0, 50, 50, 90, 90);
            path.arcTo(50, 50, 50, 50, 180, 90);
            path.arcTo(150, 50, 50, 50, 270, 90);
            path.lineTo(200, 25);
            myPolygon = path.toFillPolygon();
            break;
        case Conditional:
            myPolygon << QPointF(-100, 0) << QPointF(0, 100)
                      << QPointF(100, 0) << QPointF(0, -100)
                      << QPointF(-100, 0);
            break;
        case Layer:
            myPolygon << QPointF(-150, -100) << QPointF(150, -100)
                      << QPointF(150, 100) << QPointF(-150, 100)
                      << QPointF(-150, -100);
            break;
        default:
            myPolygon << QPointF(-120, -80) << QPointF(-70, 80)
                      << QPointF(120, 80) << QPointF(70, -80)
                      << QPointF(-120, -80);
            break;
    }
    */

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}
//! [0]
//!

void DiagramItem::setGeometry()
{
    switch (myDiagramType)
    {
    case Layer:
    {
        // these properties will be exposed
        //qreal mywidth = 150;
        //qreal myheight = 100;
        dockingLineProportion = 1;
        loopDiameterProportion = 0.3;
        qreal halfwidth = mywidth/2;
        qreal halfheight = myheight/2;

        // a rectangle
        myPolygon.clear();
        myPolygon << QPointF(-halfwidth, -halfheight) << QPointF(halfwidth, -halfheight)
                  << QPointF(halfwidth, halfheight) << QPointF(-halfwidth, halfheight)
                  << QPointF(-halfwidth, -halfheight);
        setPolygon(myPolygon);

        //myCenter = QPointF(0.0,0.0); //  relative center
        // docking lines, a cross in the middle of the box,
        horizontalDockingLine = QLineF(QPointF(-dockingLineProportion*halfwidth,0),
                                      QPointF(dockingLineProportion*halfwidth,0));
        verticalDockingLine =   QLineF(QPointF(0,dockingLineProportion*halfheight),
                                      QPointF(0,-dockingLineProportion*halfheight));
        // docking lines for dangling arrows
        QLineF incomingArrowsDockingSide = QLineF(myPolygon.at(3),myPolygon.at(2)); // lower side
        incomingArrowsDockingLine = QLineF(incomingArrowsDockingSide.pointAt(0.5*(1 - dockingLineProportion)),
                                           incomingArrowsDockingSide.pointAt(0.5*(1 + dockingLineProportion)));
        QLineF outgoingArrowsDockingSide = QLineF(myPolygon.at(0),myPolygon.at(1)); // upper side
        outgoingArrowsDockingLine = QLineF(outgoingArrowsDockingSide.pointAt(0.5*(1 - dockingLineProportion)),
                                           outgoingArrowsDockingSide.pointAt(0.5*(1 + dockingLineProportion)));

        // right side of the rectangle
        selfLoopDockingSide = QLineF(myPolygon.at(1),myPolygon.at(2));
        myLoopRotation = -180; // clockwise
        break;
    }
    case Diamond:
    {
#if 0
        // not maintained, only for test
        qreal myside = 50;
        dockingLineProportion = 0.4;
        loopDiameterProportion = 0.3;
        qreal halfdiag = myside*0.7071;
        // a diamond
        myPolygon << QPointF(-halfdiag,0) << QPointF(0,-2*halfdiag) << QPointF(halfdiag,0)
                  << QPointF(0,2*halfdiag) << QPointF(-halfdiag,0);
        setPolygon(myPolygon);

        myCenter = QPointF(0.0,0.0); //  relative center
        // docking lines, a cross in the middle of the diamond
        horizontalDockingLine = QLineF(myCenter + QPointF(-dockingLineProportion*halfdiag,0),
                                      myCenter + QPointF(dockingLineProportion*halfdiag,0));
        verticalDockingLine =   QLineF(myCenter + QPointF(0,dockingLineProportion*halfdiag),
                                      myCenter + QPointF(0,-dockingLineProportion*halfdiag));
        selfLoopDockingSide = QLineF(myPolygon.at(2),myPolygon.at(1));
        myLoopRotation = -180; // clockwise
        break;
#endif
    }
    default:
    {
        ; // add other types
    }

    }
    // compute loopBasePath, which will be shifted in loopPath()
    // according to how many selfLoops are connected to this DiagramItem.

    // This implementation is based on QPainterPath::arcTo().
    // Maybe a better one would be based on QPainterPath::quadTo().

    QRectF loopRect;
    qreal angleOnRect;
    qreal rectProportion = 1.5; // 1 < rectProportion < 2 for good result
    if (selfLoopDockingSide.dx() == 0 ||
        ::fabs(selfLoopDockingSide.dy()/selfLoopDockingSide.dx()) >1.1 ) // slightly more than 45 degrees
    {
        // impose lexicographic order on the orientation of selfLoopDockingSide,
        // here selfLoopDockingSide should point downwards
        // (remember that the y-axis points downwards too)
        if (selfLoopDockingSide.dy()<0)
            selfLoopDockingSide.setPoints(selfLoopDockingSide.p2(),selfLoopDockingSide.p1());
        // compute rectangle containing arc
//        qreal halfsize =  ::fabs(selfLoopDockingSide.dy())/2;
//        loopRect = QRectF(selfLoopDockingSide.pointAt(0.25).x()-rectProportion*halfsize/2,
//                          qMin(selfLoopDockingSide.y1(),selfLoopDockingSide.y2()),
//                          rectProportion*halfsize,halfsize);
        // size of short side of the rectangle, since rectProportion should be > 1
        qreal shortSide = ::fabs(selfLoopDockingSide.dy())*loopDiameterProportion;
        loopRect = QRectF(selfLoopDockingSide.pointAt(loopDiameterProportion/2).x()-rectProportion*shortSide/2,
                          selfLoopDockingSide.y1(),
                          rectProportion*shortSide,shortSide);
        // Angles specified in QPainterPath::arcTo() are referred to a distorted
        // geometry where 45 degrees corresponds to the angle of the diagonal
        // of the rectangle.
        angleOnRect =   atan(sin(selfLoopDockingSide.angle() * Pi / 180) /
                            (cos(selfLoopDockingSide.angle() * Pi / 180) / rectProportion))
                        * 180 / Pi;
        if (angleOnRect > 0)
            angleOnRect = angleOnRect + 180;
    }
    else
    {
        // impose lexicographic order on the orientation of selfLoopDockingSide
        // here selfLoopDockingSide should point to the right
        if (selfLoopDockingSide.dx()<0)
            selfLoopDockingSide.setPoints(selfLoopDockingSide.p2(),selfLoopDockingSide.p1());
        // compute rectangle containing arc
//        qreal halfsize =  ::fabs(selfLoopDockingSide.dx())/2;
//        loopRect = QRectF(qMin(selfLoopDockingSide.x1(),selfLoopDockingSide.x2()),
//                          selfLoopDockingSide.pointAt(0.25).y()-rectProportion*halfsize/2,
//                          halfsize,rectProportion*halfsize);
        qreal shortSide = ::fabs(selfLoopDockingSide.dx())*loopDiameterProportion;
        loopRect = QRectF(selfLoopDockingSide.x1(),
                          selfLoopDockingSide.pointAt(loopDiameterProportion/2).y()-rectProportion*shortSide/2,
                          shortSide,rectProportion*shortSide);

        angleOnRect = atan ((sin(selfLoopDockingSide.angle() * Pi / 180) / rectProportion) /
                             cos(selfLoopDockingSide.angle() * Pi / 180)  )
                        * 180 / Pi;
    }
    loopBasePath = QPainterPath();
    loopBasePath.arcMoveTo(loopRect,180 + angleOnRect);
    loopBasePath.arcTo(loopRect,180 + angleOnRect,myLoopRotation);
    loopBasePath.translate(-mywidth/2.,myheight/4.);
}

//void DiagramItem::paintLabel()
//{
//    labelItem = QGraphicsSimpleTextItem(label,this);
//    qreal halfwidth = labelItem.boundingRect().width()/2.0;
//    labelItem.setPos(myCenter - QPointF(halfwidth,0));
//}

//! [1]
void DiagramItem::removeArrow(Arrow *arrow)
{
    int index = arrows.indexOf(arrow);

    if (index != -1)
        arrows.removeAt(index);
}
//! [1]

//! [2]
void DiagramItem::removeArrows()
{
    foreach (Arrow *arrow, arrows) {
        arrow->getStartItem()->removeArrow(arrow);
        arrow->getEndItem()->removeArrow(arrow);
        scene()->removeItem(arrow);
        delete arrow;
    }
}
//! [2]

//! [3]
void DiagramItem::addArrow(Arrow *arrow)
{
    arrows.append(arrow);
}

int DiagramItem::arrowCount()
{
    return arrows.length();
}

int DiagramItem::arrowIndex(Arrow *arrow)
{
    return arrows.indexOf(arrow);
}

QList<Arrow *> DiagramItem::arrowList() const
{
    return arrows;
}
//! [3]

//! [4]
QPixmap DiagramItem::image() const
{
    QPixmap pixmap(mywidth, myheight);
  return pixmap;
}

QPointF DiagramItem::connectionPoint(Arrow *arrow) const
{
    qreal halfwidth = mywidth/2.;
    qreal halfheight = myheight/2.;
    // Use for Line type only. For SelfLoop use loopPath() instead.

    // Determine how many arrows of the type Line
    // connect arrow's startItem to endItem or viceversa.
    // Determine the index of this arrow within that set of arrows.
    if (arrow->getArrowType() != Arrow::Line)
        return QPointF(0,0);
    const DiagramItem* otherItem=0;
    DiagramItem* startItem = arrow->getStartItem();
    DiagramItem* endItem = arrow->getEndItem();
    if(this==startItem)otherItem=endItem;
    if(this==endItem)otherItem=startItem;
    int arrowCount = 0;
    int arrowIndex = 0; // starts from 1, initialised to 0 to make compiler happy
    if (!( (startItem == this && endItem != this) ||
           (startItem != this && endItem == this) )) // this should be either start or end, not both
        return QPointF(0,0);
    else if (!startItem) // incoming arrow
    {

        foreach (Arrow * other, arrowList())
        {
            if (!other->getStartItem()) // also incoming arrow
            {
                arrowCount++;
                if (arrow == other)
                    arrowIndex = arrowCount;
            }
        }
    }
    else if (!endItem) // outgoing arrow
    {
        foreach (Arrow * other, arrowList())
        {
            if (!other->getEndItem()) // also outgoing arrow
            {
                arrowCount++;
                if (arrow == other)
                    arrowIndex = arrowCount;
            }
        }
    }
    else // fully connected arrow
    {
        foreach (Arrow * other, arrowList())
        {
            if ( ((startItem == other->getStartItem() && endItem == other->getEndItem()) ||
                  (startItem == other->getEndItem() && endItem == other->getStartItem())) &&
                 other->getArrowType()== Arrow::Line)
            {
                arrowCount++;
                if (arrow == other)
                    arrowIndex = arrowCount;
            }
        }
    }
    qreal relativePointLocation=.5;
    if (arrowCount != 1)
    {
       relativePointLocation = (qreal)(arrowIndex-1)/(qreal)(arrowCount-1);
    }

    {
        if (!startItem)
            return incomingArrowsDockingLine.pointAt(relativePointLocation);
        else if (!endItem)
            return outgoingArrowsDockingLine.pointAt(relativePointLocation);
        else
        {
            // use vertical or horizontal docking line depending on arrow's angle
            if ((endItem->pos().x() - startItem->pos().x()) == 0 ||
                    ::fabs( (endItem->pos().y() - startItem->pos().y()) /
                            (endItem->pos().x() - startItem->pos().x()) ) > (halfheight/halfwidth))
            {
                QPointF rel=QPointF(0,(otherItem->center().y()>center().y())?halfheight:-halfheight);
                return horizontalDockingLine.pointAt(relativePointLocation)+rel;
            }
            else
            {
                QPointF rel=QPointF((otherItem->center().x()>center().x())?halfwidth:-halfwidth,0);
                return verticalDockingLine.pointAt(relativePointLocation)+rel;
            }
        }
    }

}

DiagramItem::Side DiagramItem::preferredSide (const Arrow *arrow)const
{
    if (!arrow->getStartItem() || !arrow->getEndItem())
        return Unset;
    if(arrow->getStartItem()==arrow->getEndItem())
        return Right;
    const DiagramItem* otherItem=0;
    qreal halfwidth = mywidth/2.;
    qreal halfheight = myheight/2.;
    if(this==arrow->getStartItem()) otherItem=arrow->getEndItem();
    if(this==arrow->getEndItem()) otherItem=arrow->getStartItem();
    if (::fabs( arrow->tangent() ) > (halfheight/halfwidth))
    { //steep
        return (otherItem->center().y()>center().y())?Top:Bottom;
    }
    else
    { //shallow
        return (otherItem->center().x()>center().x())?Right:Left;
    }

}

QPointF DiagramItem::alternativeConnectionPoint(Arrow *arrow) const
{
    qreal halfwidth = mywidth/2.;
    qreal halfheight = myheight/2.;
    // Use for Line type only. For SelfLoop use loopPath() instead.

    // Determine how many arrows of the type Line
    // connect arrow's startItem to endItem or viceversa.
    // Determine the index of this arrow within that set of arrows.
/*    if (arrow->getArrowType() != Arrow::Line)
        return QPointF(0,0);
*/    DiagramItem* startItem = arrow->getStartItem();
    DiagramItem* endItem = arrow->getEndItem();
    const DiagramItem* otherItem=0;
    if(this==arrow->getStartItem()) otherItem=arrow->getEndItem();
    if(this==arrow->getEndItem()) otherItem=arrow->getStartItem();
    Side incoming=preferredSide(arrow),old=sides[arrow];

    int arrowCount = 0;
    int arrowIndex = 1;
    foreach (Arrow * other, arrowList())
    {
            if ( preferredSide(other)==incoming)
            {
                arrowCount++;
                if(arrow==other)
                {
                    continue;

                }
                auto ooItem=(other->getStartItem()==this)?other->getEndItem():other->getStartItem();
                if(ooItem == otherItem)
                {
                    if( (void*)arrow<(void*)other )
                    {
                        arrowIndex++;
                    }
                    continue;
                }
                switch(incoming)
                {
                case(Top):
                    if ( other->cotangent() < arrow->cotangent() )
                        arrowIndex++;
                    break;
                case(Bottom):
                    if ( other->cotangent() > arrow->cotangent() )
                        arrowIndex++;
                    break;
                case(Left):
                    if ( other->tangent() < arrow->tangent() )
                        arrowIndex++;
                    break;
                case(Right):
                    if ( other->tangent() > arrow->tangent() )
                        arrowIndex++;
                    break;
                }
            }
    }
    int oldindex=indices[arrow];
    if(incoming!=old||arrowIndex!=oldindex)
        {
            sides.insert(arrow,incoming);
            indices.insert(arrow,arrowIndex);
            foreach(Arrow *other,arrowList())
            {
                auto pso=preferredSide(other);
                if(other!=arrow&&(pso==incoming||pso==old))
                {
                    other->updatePosition();
                }
            }
        }
    qreal relativePointLocation = (qreal)(arrowIndex)/(qreal)(arrowCount+1);


    if (!startItem)
            return incomingArrowsDockingLine.pointAt(relativePointLocation);
    else if (!endItem)
            return outgoingArrowsDockingLine.pointAt(relativePointLocation);
    else
    {
            // use vertical or horizontal docking line depending on arrow's angle
        switch(incoming)
        {
            case Top:
                        {
                            QPointF rel=QPointF(0,halfheight);
                            return horizontalDockingLine.pointAt(relativePointLocation)+rel;
                        }
            case Bottom:
                        {
                            QPointF rel=QPointF(0,-halfheight);
                            return horizontalDockingLine.pointAt(relativePointLocation)+rel;
                        }
            case Right:
            {
                QPointF rel=QPointF(halfwidth,0);
                return verticalDockingLine.pointAt(relativePointLocation)+rel;
            }
            case Left:
            {
                QPointF rel=QPointF(-halfwidth,0);
                return verticalDockingLine.pointAt(relativePointLocation)+rel;
            }

        }
    }

}

QPainterPath DiagramItem::loopPath(Arrow *arrow) const
{
    return loopBasePath.translated(alternativeConnectionPoint(arrow));
    // Use only with arrows of type SelfLoop.
    // similarly to connectionPoint, compute arrowCount and arrowIndex
    if (arrow->getArrowType() != Arrow::SelfLoop)
        return QPainterPath();
    int arrowCount = 0;
    int arrowIndex = 0; // starts from 1, initialised to 0 to make compiler happy
    QSet<Arrow*>ArrowSeen;
    foreach (Arrow * other, arrowList())
    {
        if(ArrowSeen.contains(other)) continue;
        ArrowSeen << other;
        if ( this == other->getStartItem() && this == other->getEndItem() )
        {
            arrowCount++;
            if (arrow == other)
                arrowIndex = arrowCount;
        }
    }
    // loopBasePath start at selfLoopDockingSide.p1().
    // It has to be translated along selfLoopDockingSide to an approptiate position.
    if (arrowCount == 1)
    {
        //return loopBasePath.translated(0.25*selfLoopDockingSide.dx(),
        //                               0.25*selfLoopDockingSide.dy());
        return loopBasePath.translated((0.5-loopDiameterProportion/2)*selfLoopDockingSide.dx(),
                                       (0.5-loopDiameterProportion/2)*selfLoopDockingSide.dy());
    }
    else
    {
        // the placement of selfloops is optimised for two,
        // where one ends in the center of the other
        // (that's where 0.75 comes from)
        qreal relativePointLocation = (qreal)(arrowIndex-1)/(qreal)(arrowCount-1);
        //qreal relativeLocationFirstPoint = 0.5 * ((1-dockingLineProportion)/2 + relativePointLocation * dockingLineProportion);
        qreal relativeLocationFirstPoint = (0.5 - 0.75*loopDiameterProportion) +
                relativePointLocation * loopDiameterProportion/2;
        return loopBasePath.translated(relativeLocationFirstPoint*selfLoopDockingSide.dx(),
                                       relativeLocationFirstPoint*selfLoopDockingSide.dy());
    }
}

void DiagramItem::setLabel(QString _label)
{
    label = _label;
    update();
}

void DiagramItem::read(const QJsonObject &json)
{
    myName = json["name"].toString();
    QPointF position(json["x"].toDouble(),json["y"].toDouble());
    if (!position.isNull())
        setPos(position);
}

void DiagramItem::write(QJsonObject &json) const
{
    json["name"] = myName;
    QPointF position = pos();
    json["x"] = position.x();
    json["y"] = position.y();
}

void DiagramItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
//    Q_UNUSED(option)
//    Q_UNUSED(widget)
    // check if label fits
    QRectF * minBoundingRect = new QRectF;
    painter->drawText(boundingRect(), Qt::AlignCenter | Qt::TextSingleLine, label,minBoundingRect);
    if (boundingRect().width() < minBoundingRect->width() + 2 * BOUNDINGRECTPADDING)
    {
        prepareGeometryChange();
//        mywidth = minBoundingRect->width() + 2 * BOUNDINGRECTPADDING;
        setGeometry();
    }


    m_pen.setWidth(penWidth);

    setPen(m_pen);
    auto moption=*option;
    moption.state&= ~QStyle::State_Selected;
    QGraphicsPolygonItem::paint(painter,& moption, widget);
    paintLabel(painter);
}

void DiagramItem::paintLabel(QPainter *painter)
{
    // inspired by Dunnart ShapeObj::paintLabel
    painter->setPen(Qt::black);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    //QRectF * minBoundingRect = new QRectF;
    painter->drawText(boundingRect(), Qt::AlignCenter | Qt::TextSingleLine, label);

    //setSize(boundingRect().size().expandedTo(expandRect(*minBoundingRect,BOUNDINGRECTPADDING).size()));
    //delete minBoundingRect;

}
//! [4]





//! [5]
void DiagramItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    myContextMenu->exec(event->screenPos());
}
//! [5]

//! [6]
QVariant DiagramItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        foreach (Arrow *arrow, arrows) {
            arrow->updatePosition();
        }
    }

    return value;
}
