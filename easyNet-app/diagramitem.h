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

#ifndef DIAGRAMITEM_H
#define DIAGRAMITEM_H

#include <QGraphicsPixmapItem>
#include <QList>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsItem;
class QGraphicsScene;
class QTextEdit;
class QGraphicsSceneMouseEvent;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPolygonF;
template <> class QPair<QPointF,QPointF>;
QT_END_NAMESPACE

class Arrow;

//! [0]
class DiagramItem : public QGraphicsPolygonItem
{
public:
    enum { Type = UserType + 15 };
    enum DiagramType { Layer, Diamond };

    DiagramItem(DiagramType diagramType, QString name, QMenu *contextMenu, QGraphicsItem *parent = 0);
    //DiagramItem(QMenu *contextMenu, QGraphicsItem *parent = 0);


    void removeArrow(Arrow *arrow);
    void removeArrows();
    DiagramType diagramType() const { return myDiagramType; }
    QPolygonF polygon() const { return myPolygon; }
    void addArrow(Arrow *arrow);
    int arrowCount();
    int arrowIndex(Arrow *arrow);
    QList<Arrow *> arrowList() const;
    QPixmap image() const;
    int type() const { return Type;}
    QPointF const center() {return myCenter; }
    QPointF  connectionPoint(Arrow *arrow) const;
    QPainterPath  loopPath(Arrow *arrow) const;
    qreal loopRotation() const {return myLoopRotation;}
    QString name() const {return myName;}
    void setLabel(QString _label);
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option, QWidget *widget);
    void paintLabel(QPainter *painter);



protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void setGeometry();
    //void paintLabel();


//private:
    DiagramType myDiagramType;
    QPolygonF myPolygon;
    QPointF myCenter;
    QMenu *myContextMenu;
    QList<Arrow *> arrows;
    QLineF horizontalDockingLine;
    QLineF verticalDockingLine;
    QLineF incomingArrowsDockingLine;
    QLineF outgoingArrowsDockingLine;
    QLineF selfLoopDockingSide;
    qreal myLoopRotation; // + or - 180 degrees
    qreal dockingLineProportion;
    qreal loopDiameterProportion;
    QPainterPath loopBasePath;
    QString label = "";
    QString myName;
    QGraphicsSimpleTextItem labelItem;
    const double BOUNDINGRECTPADDING = 10;
    qreal mywidth = 150;
    qreal myheight = 100;
    QColor myColor;
    int penWidth;


};
//! [0]

#endif // DIAGRAMITEM_H