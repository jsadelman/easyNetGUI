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


#include "arrow.h"

#include <math.h>

#include <QPen>
#include <QPainter>
#include <QPainterPathStroker>
#include <QDebug>


const qreal Pi = 3.14;

//! [0]
Arrow::Arrow(DiagramItem *startItem, DiagramItem *endItem, ArrowTipType arrowTipType, QGraphicsItem *parent)
    :  QGraphicsPathItem(parent)
{
    myStartItem = startItem;
    myEndItem = endItem;
    if (myStartItem == myEndItem)
        myArrowType = SelfLoop;
    else
        myArrowType = Line;
    myArrowTipType = arrowTipType;
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    myColor = Qt::black;
    penWidth = 3;
    arrowSize = 15;
    setPen(QPen(myColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}
//! [0]

//! [1]
QRectF Arrow::boundingRect() const
{
    qreal extra = (pen().width() + 20) / 2.0;
    return path().boundingRect().adjusted(-extra, -extra, extra, extra);
}
//! [1]

//! [2]
QPainterPath Arrow::shape() const
{
    QPainterPath shapePath;
    shapePath.addPath(path());/*
    // it looks like adding the arrow tip does not influence the selection area.
    switch (myArrowTipType)
    {
    case Excitatory:
        shapePath.addPolygon(arrowHead);
        break;
    case Inhibitory:
        shapePath.addEllipse(headCenter, arrowSize / 2, arrowSize / 2);
        break;
    }*/
    QPainterPathStroker stroker;
    stroker.setWidth(20);

    return stroker.createStroke(shapePath);


}
//! [2]

//! [3]
void Arrow::updatePosition()
{
    // any time an arrow is to be moved, this function resets path, which in turn
    // is used by boundingRect() and shape() to determine e.g.
    // the area that has to be repainted (boundingRect) and the selected area (shape).
    // boundingRect() and shape() are not called by the user but managed by the Scene.
    switch (myArrowType)
    {
    case Line:
    {
        QPainterPath updatePath(mapFromItem(myStartItem,myStartItem->connectionPoint(this)));
        updatePath.lineTo(mapFromItem(myEndItem,myEndItem->connectionPoint(this)));
        setPath(updatePath);
        break;
    }
    case SelfLoop:
    {
        QPainterPath updatePath(mapFromItem(myStartItem,myStartItem->loopPath(this)));
        setPath(updatePath);
        break;
    }
    }
}
//! [3]

//! [4]
void Arrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
          QWidget *)
{
    if (myArrowType == Line && myStartItem->collidesWithItem(myEndItem))
        return;

    QPen myPen = pen();
    myPen.setColor(myColor);
    myPen.setWidth(penWidth);
    painter->setPen(myPen);
    painter->setBrush(Qt::NoBrush);
//! [4] //! [5]

    switch (myArrowType)
    {
    case Line:
    {
        QPointF startPoint = myStartItem->pos() + myStartItem->connectionPoint(this);
        QLineF connectLine = QLineF(startPoint, myEndItem->pos() + myEndItem->connectionPoint(this));
        QPolygonF endPolygon = myEndItem->polygon();
        QPointF p1 = endPolygon.first() + myEndItem->pos();
        QPointF p2;
        QPointF intersectPoint;
        QLineF polyLine;
        for (int i = 1; i < endPolygon.count(); ++i) {
            p2 = endPolygon.at(i) + myEndItem->pos();
            polyLine = QLineF(p1, p2);
            QLineF::IntersectType intersectType =
                    polyLine.intersect(connectLine, &intersectPoint);
            if (intersectType == QLineF::BoundedIntersection)
                break;
            p1 = p2;
        }

        setLine(QLineF(intersectPoint, startPoint));
        QPainterPath arrowPath(startPoint);
        arrowPath.lineTo(intersectPoint);
        setPath(arrowPath);
        switch (myArrowTipType) {
        case Excitatory:
        {
            // this trig calculation is from the diagramscene example
            double angle = ::acos(line().dx() / line().length());
            if (line().dy() >= 0)
                angle = (Pi * 2) - angle;

            QPointF arrowP1 = line().p1() + QPointF(sin(angle + Pi / 3) * arrowSize,
                                                    cos(angle + Pi / 3) * arrowSize);
            QPointF arrowP2 = line().p1() + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                                    cos(angle + Pi - Pi / 3) * arrowSize);

            arrowHead.clear();
            arrowHead << line().p1() << arrowP1 << arrowP2;
            //! [6] //! [7]
            painter->drawPath(path());
            painter->setBrush(myColor);
            painter->drawPolygon(arrowHead);
            break;
        }
        case Inhibitory:
        {
            headCenter = line().p1() + QPointF(line().dx() / line().length(),
                                               line().dy() / line().length()) *
                    arrowSize / 2;
            painter->drawPath(path());
            painter->setBrush(myColor);
            painter->drawEllipse(headCenter, arrowSize / 2, arrowSize / 2);
            break;
        }
        }
        break;
    }
    case SelfLoop:
    {
        QPainterPath arrowPath = myStartItem->loopPath(this);
        arrowPath.translate(myStartItem->pos());
        setPath(arrowPath);
        switch (myArrowTipType) {
        case Excitatory:
        {
            qreal tangentAngle = (path().angleAtPercent(1) + 180) * Pi / 180;
            QPointF arrowP1 = path().currentPosition() +
                              QPointF(cos(tangentAngle + Pi/6), -sin(tangentAngle + Pi/6)) * arrowSize;
            QPointF arrowP2 = path().currentPosition() +
                              QPointF(cos(tangentAngle - Pi/6), -sin(tangentAngle - Pi/6)) * arrowSize;
            arrowHead.clear();
            arrowHead << path().currentPosition() << arrowP1 << arrowP2;
            painter->drawPath(path());
            painter->setBrush(myColor);
            painter->drawPolygon(arrowHead);
            break;
        }
        case Inhibitory:
        {
            qreal tangentAngle = (::fabs(path().angleAtPercent(1)) + 180) * Pi / 180;
            headCenter = path().currentPosition() +
                         QPointF(cos(tangentAngle), sin(tangentAngle)) * arrowSize / 2;
            painter->drawPath(path());
            painter->setBrush(myColor);
            painter->drawEllipse(headCenter, arrowSize / 2, arrowSize / 2);
            break;
        }
        }
    }
    }

//! [5] //! [6]


        if (isSelected())
        {
            QColor selectionColor = QColor("yellow");
            selectionColor.setAlpha(100);
            painter->setPen(QPen(selectionColor,6,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(path());

        }
}


//! [7]
