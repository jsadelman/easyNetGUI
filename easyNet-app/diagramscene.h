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

#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include "diagramitem.h"
#include "diagramtextitem.h"
#include "arrow.h"

#include "libdunnartcanvas/canvas.h"

namespace dunnart {
    class CanvasItem;
    class ShapeObj;
}

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QGraphicsSceneContextMenuEvent;
class QMenu;
class QAction;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QGraphicsTextItem;
class QGraphicsItem;
class QColor;
class AsLazyNutObject;
class ObjectCache;
class ObjectCacheFilter;
class DescriptionUpdater;
class QDomDocument;

class Box;

QT_END_NAMESPACE

//! [0]
class DiagramScene : public dunnart::Canvas
{
    Q_OBJECT
    Q_PROPERTY(QString boxType READ boxType)
    Q_PROPERTY(QString arrowType READ arrowType)
    Q_PROPERTY(QString baseName READ baseName WRITE setBaseName)
    Q_PROPERTY(QString layoutFile READ layoutFile WRITE setLayoutFile)

    Q_PROPERTY(bool newModelLoaded READ newModelLoaded WRITE setNewModelLoaded)

public:
//    enum Mode { InsertItem, InsertLine, InsertText, MoveItem };

    explicit DiagramScene(QString box_type, QString arrow_type);
    QString boxType() {return m_boxType;}
    QString arrowType() {return m_arrowType;}
//    void setObjCatalogue(ObjectCache *catalogue);
//    QFont font() const { return myFont; }
//    QColor textColor() const { return myTextColor; }
//    QColor itemColor() const { return myItemColor; }
//    QColor lineColor() const { return myLineColor; }
//    void setLineColor(const QColor &color);
//    void setTextColor(const QColor &color);
//    void setItemColor(const QColor &color);
//    void setFont(const QFont &font);
    void read(const QJsonObject &json);
    void write(QJsonObject &json);
    QString baseName() {return m_baseName;}
    void setBaseName(QString baseName);
    QString layoutFile() {return m_layoutFile;}
    void setLayoutFile(QString layoutFile) {m_layoutFile = layoutFile;}
    bool newModelLoaded() {return m_newModelLoaded;}
    void setNewModelLoaded(bool isNew) {m_newModelLoaded = isNew;}
    bool validForAlignment(QList<dunnart::CanvasItem *> items);


    QList<QSet<dunnart::ShapeObj *> > connectedComponents();
    QList<dunnart::ShapeObj *> shapes();
    QList<Box *> boxes();

public slots:
//    void setMode(Mode mode);
//    void setItemType(DiagramItem::DiagramType type);
//    void setArrowTipType(Arrow::ArrowTipType type);
//    void editorLostFocus(DiagramTextItem *item);
//    void syncToObjCatalogue();
    void setSelected(QString name);
//    void savedLayoutToBeLoaded(QString _savedLayout);
//    void prepareToLoadLayout(QString fileName);

    void initShapePlacement();
    void wakeUp();
    void goToSleep();

signals:
    void itemInserted(DiagramItem*);
    void textInserted(QGraphicsTextItem*);
    void itemSelected(QGraphicsItem*);
    void objectSelected(QString);
    void initArrangement();
//    void showObj(LazyNutObj * obj, LazyNutObjCatalogue* objectCatalogue);
//    void layoutSaveAttempted();

    void animationFinished();
    void createNewPlotOfType(QString name, QString type,
                             QMap<QString, QString> _defaultSettings=QMap<QString,QString>());
    void plotDestroyed(QString name);



protected:
//    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
//    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
//    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
//    void contextMenuEvent(QGraphicsSceneContextMenuEvent *mouseEvent);

private slots:
    void positionObject(QString name, QString type, QDomDocument* domDoc);
    void removeObject(QString name);
    void renderObject(QDomDocument* domDoc);



    void syncToObjCatalogue();
//    void enableObserverClicked();
//    void disableObserverClicked();
//    void lesionClicked();
private:
    void render();
//    bool isItemChange(int type);

    bool awake;

    ObjectCache *objectCatalogue;
    QHash<QString, dunnart::CanvasItem*> itemHash;
//    ObjectCatalogueFilter *objectFilter;
    DescriptionUpdater *descriptionUpdater;
    ObjectCacheFilter *boxFilter;
    ObjectCacheFilter *arrowFilter;
    DescriptionUpdater *arrowDescriptionUpdater;
    DescriptionUpdater *boxDescriptionUpdater;
    QList<QDomDocument*> renderList;

    QAction* enableObserverAction;
    QAction* disableObserverAction;
    QAction* lesionAction;




    QString m_boxType;
    QString m_arrowType;
    QStringList connections;
    QString m_baseName;
    QString m_layoutFile;

    bool m_newModelLoaded;

//    QString savedLayout;
//    bool layoutLoaded = false;
//    bool layoutChanged;
//    DiagramItem::DiagramType myItemType;
//    Arrow::ArrowTipType myArrowTipType;
//    QMenu *myItemMenu;
//    Mode myMode;
//    bool leftButtonDown;
    //QPointF startPoint;
    QPointF defaultPosition;
    QPointF currentPosition;
    QPointF itemOffset;
    QPointF arrowOffset;
//    QGraphicsLineItem *line;
//    QFont myFont;
//    DiagramTextItem *textItem;
//    QColor myTextColor;
//    QColor myItemColor;
//    QColor myLineColor;
    QString selectedObject;

    QString boxLongNameToDisplayIntact;
    qreal boxWidthMarginProportionToLongestLabel;
    qreal boxWidthOverHeight;

    qreal jitter;

    friend class DiagramWindow; // dunnart::Canvas has lots of friends, this is the same idea.
};
//! [0]

#endif // DIAGRAMSCENE_H
