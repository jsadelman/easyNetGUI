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
Arrow::Arrow(QString name, DiagramItem *startItem, DiagramItem *endItem, ArrowTipType arrowTipType, QGraphicsItem *parent)
    :  name(name), startItem(startItem), endItem(endItem), arrowTipType(arrowTipType), QGraphicsPathItem(parent)
{
    arrowStart = QPoint();
    if (startItem == endItem)
        arrowType = SelfLoop;
    else
        arrowType = Line;

    init();
}

Arrow::Arrow(QString name, QPointF startingPoint, Arrow::ArrowTipType arrowTipType, QGraphicsItem *parent)
    :  name(name), arrowStart(startingPoint), arrowTipType(arrowTipType), QGraphicsPathItem(parent)
{
    startItem = nullptr;
    endItem = nullptr;
    arrowType = Line;
    init();
}

Arrow::Arrow(QString name, ArrowTipType arrowTipType, QGraphicsItem *parent)
   :  name(name), arrowTipType(arrowTipType), QGraphicsPathItem(parent)
{
    startItem = nullptr;
    endItem = nullptr;
    arrowType = Line;
    arrowStart = QPointF();
    init();
}

Arrow::Arrow(ArrowTipType arrowTipType, QGraphicsItem *parent)
   : arrowTipType(arrowTipType), QGraphicsPathItem(parent)
{
    startItem = nullptr;
    endItem = nullptr;
    name = QString();
    arrowType = Line;
    arrowStart = QPointF();
    init();
}

void Arrow::init()
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    defaultLength = 100;
    myColor = Qt::black;
    penWidth = 2;
    arrowSize = 10;
    setPen(QPen(myColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}


QRectF Arrow::boundingRect() const
{
    qreal extra = (pen().width() + 20) / 2.0;
    return path().boundingRect().adjusted(-extra, -extra, extra, extra);
}

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

void Arrow::updatePosition()
{
    // any time an arrow is to be moved, this function resets path, which in turn
    // is used by boundingRect() and shape() to determine e.g.
    // the area that has to be repainted (boundingRect) and the selected area (shape).
    // boundingRect() and shape() are not called by the user but managed by the Scene.
    switch (arrowType)
    {
    case Line:
    {
        QPainterPath updatePath;
        if (!startItem && !endItem)
        {
            arrowEnd = arrowStart - QPointF(0, defaultLength);
            updatePath.moveTo(arrowStart);
            updatePath.lineTo(arrowEnd);
        }
        else if (!startItem)
        {
            arrowEnd = mapFromItem(endItem,endItem->connectionPoint(this));
            arrowStart = arrowEnd + QPointF(0, defaultLength);
            updatePath.moveTo(arrowStart);
            updatePath.lineTo(arrowEnd);
        }
        else if (!endItem)
        {
            arrowStart = mapFromItem(startItem,startItem->connectionPoint(this));
            arrowEnd = arrowStart - QPointF(0, defaultLength);
            updatePath.moveTo(arrowStart);
            updatePath.lineTo(arrowEnd);
        }
        else
        {
            updatePath.moveTo(mapFromItem(startItem,startItem->connectionPoint(this)));
            updatePath.lineTo(mapFromItem(endItem,endItem->connectionPoint(this)));
        }
        setPath(updatePath);
        break;
    }
    case SelfLoop:
    {
        QPainterPath updatePath(mapFromItem(startItem,startItem->loopPath(this)));
        setPath(updatePath);
        break;
    }
    }
}

void Arrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
          QWidget *)
{
    QPen myPen = pen();
    myPen.setColor(myColor);
    myPen.setWidth(penWidth);
    painter->setPen(myPen);
    painter->setBrush(Qt::NoBrush);

    switch (arrowType)
    {
    case Line:
    {
        if (!startItem && !endItem)
        {
            arrowEnd = arrowStart - QPointF(0, defaultLength);
        }
        else if (!startItem)
        {
            arrowEnd = endItem->connectionPoint(this) + endItem->pos();
            arrowStart = arrowEnd + QPointF(0, defaultLength);
        }
        else if (!endItem)
        {
            arrowStart = startItem->connectionPoint(this) + startItem->pos();
            arrowEnd = arrowStart - QPointF(0, defaultLength);
        }
        else
        {
            if (startItem->collidesWithItem(endItem))
                return;
            arrowStart = startItem->connectionPoint(this) + startItem->pos();
            QLineF connectLine = QLineF(arrowStart, endItem->connectionPoint(this) + endItem->pos());
            QPolygonF endPolygon = endItem->polygon();
            QPointF p1 = endPolygon.first() + endItem->pos();
            QPointF p2;
            QPointF intersectPoint;
            QLineF polyLine;
            for (int i = 1; i < endPolygon.count(); ++i) {
                p2 = endPolygon.at(i) + endItem->pos();
                polyLine = QLineF(p1, p2);
                QLineF::IntersectType intersectType =
                        polyLine.intersect(connectLine, &intersectPoint);
                if (intersectType == QLineF::BoundedIntersection)
                    break;
                p1 = p2;
            }
            arrowEnd = intersectPoint;
        }
        QPainterPath arrowPath(arrowStart);
        arrowPath.lineTo(arrowEnd);
        setPath(arrowPath);
        break;
    }
    case SelfLoop:
    {
        QPainterPath arrowPath = startItem->loopPath(this);
        arrowPath.translate(startItem->pos());
        setPath(arrowPath);
        break;
    }
    default:
        return;
    }
    qreal reverseAngle = (path().angleAtPercent(1) + 180) * Pi / 180;

    switch (arrowTipType) {
    case Excitatory:
    {
        QPointF arrowP1 = path().currentPosition() +
                QPointF(cos(reverseAngle + Pi/6), -sin(reverseAngle + Pi/6)) * arrowSize;
        QPointF arrowP2 = path().currentPosition() +
                QPointF(cos(reverseAngle - Pi/6), -sin(reverseAngle - Pi/6)) * arrowSize;
        arrowHead.clear();
        arrowHead << path().currentPosition() << arrowP1 << arrowP2;
        painter->drawPath(path());
        painter->setBrush(myColor);
        painter->drawPolygon(arrowHead);
        break;
    }
    case Inhibitory:
    {
        headCenter = path().currentPosition() +
                QPointF(cos(reverseAngle), -sin(reverseAngle)) * arrowSize / 2;
        painter->drawPath(path());
        painter->setBrush(myColor);
        painter->drawEllipse(headCenter, arrowSize / 2, arrowSize / 2);
        break;
    }
    default:
        painter->drawPath(path());
    }

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
