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

#include "diagramscene.h"
#include "arrow.h"
#include "lazynutobject.h"
#include "objectcataloguefilter.h"
#include "descriptionupdater.h"
#include "sessionmanager.h"

#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/dunnart_connector.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/graphlayout.h"


#include <algorithm>


#include <QTextCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QMetaObject>
#include <QDomDocument>
#include <QDebug>

using dunnart::CanvasItem;
using dunnart::RectangleShape;
using dunnart::ShapeObj;
using dunnart::Connector;

Q_DECLARE_METATYPE(QDomDocument*)

//! [0]
DiagramScene::DiagramScene(QString boxType, QString arrowType)
    : boxType(boxType), arrowType(arrowType), Canvas()
{
    selectedObject = "";
//    myItemMenu = itemMenu;
//    myMode = MoveItem;
//    myItemType = DiagramItem::Layer;
//    line = 0;
//    textItem = 0;
//    myItemColor = Qt::white;
//    myTextColor = Qt::black;
//    myLineColor = Qt::black;
    defaultPosition = QPointF(300,125); //  150);
    currentPosition = defaultPosition;
    itemOffset = QPointF(0,50) ; // 150);
    arrowOffset = QPointF(50,0);

    objectFilter = new ObjectCatalogueFilter(this);
    objectFilter->setTypeList(QStringList({boxType, arrowType}));
    descriptionUpdater = new DescriptionUpdater(this);
    descriptionUpdater->setProxyModel(objectFilter);


    connect(objectFilter, SIGNAL(objectCreated(QString, QString, QDomDocument*)),
            this, SLOT(positionObject(QString, QString, QDomDocument*)));
    connect(objectFilter, SIGNAL(objectDestroyed(QString)),
            this, SLOT(removeObject(QString)));
    connect(descriptionUpdater, SIGNAL(descriptionUpdated(QDomDocument*)),
            this, SLOT(renderObject(QDomDocument*)));
    connect(this, SIGNAL(wakeUp()), descriptionUpdater,SLOT(wakeUpUpdate()));
    connect(this, SIGNAL(wakeUp()), this,SLOT(syncToObjCatalogue()));
    connect(this, SIGNAL(goToSleep()), descriptionUpdater,SLOT(goToSleep()));


}


// This implementation of the connected component algorithm is adapted from:
// https://breakingcode.wordpress.com/2013/04/08/finding-connected-components-in-a-graph/
QList<QSet<ShapeObj *> > DiagramScene::connectedComponents()
{
    QList<QSet<ShapeObj *> > cc;
    QSet<ShapeObj *> shapeSet = shapes().toSet();
    while (!shapeSet.isEmpty())
    {
        ShapeObj * shape = shapeSet.toList().first();
        shapeSet.remove(shape);
        QSet<ShapeObj *> group({shape});
        QList<ShapeObj *> queue({shape});
        while (!queue.isEmpty())
        {
            QSet<ShapeObj *> neighbourSet = queue.takeFirst()->neighbours();
            neighbourSet.subtract(group);
            shapeSet.subtract(neighbourSet);
            group.unite(neighbourSet);
            queue.append(neighbourSet.toList());
        }
        cc.append(group);
    }
    return cc;
}

QList<ShapeObj *> DiagramScene::shapes()
{
    QList<ShapeObj *> result;
    QListIterator<CanvasItem*> it(items());
    while(it.hasNext())
    {
        ShapeObj *shape = qobject_cast<ShapeObj*>(it.next());
        if (shape)
            result.append(shape);
    }
    return result;
}

#if 0

void DiagramScene::setObjCatalogue(ObjectCatalogue *catalogue)
{
    objectCatalogue = catalogue;
}
//! [0]

//! [1]
void DiagramScene::setLineColor(const QColor &color)
{
    myLineColor = color;
    if (isItemChange(Arrow::Type)) {
        Arrow *item = qgraphicsitem_cast<Arrow *>(selectedItems().first());
        item->setColor(myLineColor);
        update();
    }
}
//! [1]

//! [2]
void DiagramScene::setTextColor(const QColor &color)
{
    myTextColor = color;
    if (isItemChange(DiagramTextItem::Type)) {
        DiagramTextItem *item = qgraphicsitem_cast<DiagramTextItem *>(selectedItems().first());
        item->setDefaultTextColor(myTextColor);
    }
}
//! [2]

//! [3]
void DiagramScene::setItemColor(const QColor &color)
{
    myItemColor = color;
    if (isItemChange(DiagramItem::Type)) {
        DiagramItem *item = qgraphicsitem_cast<DiagramItem *>(selectedItems().first());
        item->setBrush(myItemColor);
    }
}
//! [3]

//! [4]
void DiagramScene::setFont(const QFont &font)
{
    myFont = font;

    if (isItemChange(DiagramTextItem::Type)) {
        QGraphicsTextItem *item = qgraphicsitem_cast<DiagramTextItem *>(selectedItems().first());
        //At this point the selection can change so the first selected item might not be a DiagramTextItem
        if (item)
            item->setFont(myFont);
    }
}

void DiagramScene::read(const QJsonObject &json)
{
    QJsonArray itemArray = json["diagramItems"].toArray();
    for (int itemIndex = 0; itemIndex < itemArray.size(); ++itemIndex)
    {
        QJsonObject itemObject = itemArray[itemIndex].toObject();
        QString name = itemObject["name"].toString();
        if (itemHash.contains(name))
        {
            QGraphicsItem * item = itemHash.value(name);
            if (item->type() == DiagramItem::Type) // it should always be true
            {
                DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem *>(item);
                diagramItem->read(itemObject);
            }
        }
    }
}

void DiagramScene::write(QJsonObject &json) const
{
    QJsonArray itemArray;
    foreach (QGraphicsItem * item, items())
    {
        if (item->type() == DiagramItem::Type)
        {
            DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem *>(item);
            QJsonObject itemObject;
            diagramItem->write(itemObject);
            itemArray.append(itemObject);
        }
    }
    json["diagramItems"] = itemArray;
}
//! [4]

void DiagramScene::setMode(Mode mode)
{
    myMode = mode;
}

void DiagramScene::setItemType(DiagramItem::DiagramType type)
{
    myItemType = type;
}

void DiagramScene::setArrowTipType(Arrow::ArrowTipType type)
{
    myArrowTipType = type;
}

//! [5]
void DiagramScene::editorLostFocus(DiagramTextItem *item)
{
    QTextCursor cursor = item->textCursor();
    cursor.clearSelection();
    item->setTextCursor(cursor);

    if (item->toPlainText().isEmpty()) {
        removeItem(item);
        item->deleteLater();
    }
}

#endif
//void DiagramScene::syncToObjCatalogue()
//{
//    if (objectCatalogue == nullptr)
//        return;
//    // display new layers, hold new connections in a list
//    QStringList newConnections{};
//    foreach(QString name, objectCatalogue->keys().toSet() - itemHash->keys().toSet())
//    {
//        if (objectCatalogue->value(name)->type == "layer")
//        {
//            DiagramItem *item = new DiagramItem(DiagramItem::Layer, name, myItemMenu);
//            //item->setLabel(name);
//            item->setBrush(myItemColor);
//            addItem(item);
//            item->setPos(currentPosition);
//            currentPosition += itemOffset;
//            emit itemInserted(item);
//            itemHash->insert(name,item);
//        }
//        else if (objectCatalogue->value(name)->type == "connection")
//            newConnections << name;
//    }
//    // display new connections
//    foreach(QString name, newConnections)
//    {
//        DiagramItem *startItem = qgraphicsitem_cast<DiagramItem *>
//                    (itemHash->value(objectCatalogue->value(name)->getValue("Source")));
//        DiagramItem *endItem   = qgraphicsitem_cast<DiagramItem *>
//                    (itemHash->value(objectCatalogue->value(name)->getValue("Target")));
//        // arrows without start and end are not plotted in this version
//        if (!(startItem && endItem))
//            continue;
//        Arrow *arrow = new Arrow(name, startItem, endItem, Arrow::Excitatory);
//        arrow->setColor(myLineColor);
//        startItem->addArrow(arrow);
//        if (startItem != endItem)
//            endItem->addArrow(arrow);
//        arrow->setZValue(-1000.0);
//        addItem(arrow);
//        itemHash->insert(name,arrow);
//    }
//    // remove deleted objects
//    // (see DesignWindow::deleteItem)
//    foreach (QString name, itemHash->keys().toSet() - objectCatalogue->keys().toSet())
//    {
//        QGraphicsItem* item = itemHash->value(name);
//        if (item->type() == Arrow::Type)
//        {
//            removeItem(item);
//            Arrow *arrow = qgraphicsitem_cast<Arrow *>(item);
//            arrow->getStartItem()->removeArrow(arrow);
//            arrow->getEndItem()->removeArrow(arrow);
//            delete item;
//            itemHash->remove(name);
//        }
//    }
//    foreach (QString name, itemHash->keys().toSet() - objectCatalogue->keys().toSet())
//    {
//        QGraphicsItem* item = itemHash->value(name);
//        if (item->type() == DiagramItem::Type)
//        {
//            qgraphicsitem_cast<DiagramItem *>(item)->removeArrows();
//            removeItem(item);
//            delete item;
//            itemHash->remove(name);
//        }
//    }
//    if (!layoutLoaded)
//    {
//        QFile savedLayoutFile(savedLayout);
//        if (savedLayoutFile.open(QIODevice::ReadOnly))
//        {
//            QByteArray savedLayoutData = savedLayoutFile.readAll();
//            QJsonDocument savedLayoutDoc(QJsonDocument::fromJson(savedLayoutData));
//            read(savedLayoutDoc.object());
//        }
//        //layoutLoaded = true;
//    }
//}

#if 0
void DiagramScene::prepareToLoadLayout(QString fileName)
{
    savedLayout = fileName;
//    connect(descriptionUpdater, SIGNAL(descriptionUpdated(QDomDocument*)),
//            this, SLOT(loadLayout()));
}

void DiagramScene::loadLayout()
{
    if (!objectFilter->isAllValid())
        return;

//    if (!layoutLoaded)
    {
        QFile savedLayoutFile(savedLayout);
        if (savedLayoutFile.open(QIODevice::ReadOnly))
        {
            QByteArray savedLayoutData = savedLayoutFile.readAll();
            QJsonDocument savedLayoutDoc(QJsonDocument::fromJson(savedLayoutData));
            read(savedLayoutDoc.object());
            layoutChanged=false;
        }
        //layoutLoaded = true;
    }

//    disconnect(descriptionUpdater, SIGNAL(descriptionUpdated(QDomDocument*)),
//                   this, SLOT(loadLayout()));
}
#endif
void DiagramScene::setSelected(QString name)
{
    qDebug() << "setSelected" << name;
    if (itemHash.contains(name))
        setSelection(QList<CanvasItem*>{itemHash.value(name)});
}

void DiagramScene::initShapePlacement()
{
    // Connected component (CC) centres will be placed on a square grid of S points,
    // where S is the square number closest to the number of CCs.
    // The distance between CCs is estimated by supposing that the nodes in a CC will be
    // displayed in a square grid of S points, where S is defined as above.
    // The distance between nodes is taken as the ideal distance parameter.
    // The distance between CCs is computed based on the CC that contains the largest
    // number of nodes.

    QList<QSet<ShapeObj *> > cc = connectedComponents();
    int maxCCsize = (*std::max_element(cc.begin(), cc.end(),
                                      [=](QSet<ShapeObj *> s1, QSet<ShapeObj *> s2){
        return s1.size() < s2.size();
    })).size();
    int ccGridSize = qCeil(qSqrt(maxCCsize));
    int sceneGridSize = qCeil(qSqrt(cc.length()));
    qreal length = property("idealEdgeLengthModifier").toDouble() * idealConnectorLength();
    qreal ccGridLength = ccGridSize * length;

    QList<QPointF> gridPoints;
    for (int i = 0; i < sceneGridSize; ++i)
        for (int j = 0; j < sceneGridSize; ++j)
            gridPoints.append(QPointF(j * ccGridLength, i * ccGridLength));

    foreach(QSet<ShapeObj *> c, cc)
    {
        QPointF point = gridPoints.takeFirst();
        foreach (ShapeObj * box, c)
            box->setCentrePos(point +
                              QPointF((double)(qrand() % 10 - 5)*30, (double)(qrand() % 10 - 5)*30));
    }
    layout()->initialise();
    updateConnectorsForLayout();

}


#if 0
void DiagramScene::savedLayoutToBeLoaded(QString _savedLayout)
{
    savedLayout = _savedLayout;
    layoutLoaded = !(QFileInfo(savedLayout).exists());
}

void DiagramScene::saveLayout()
{
//    if (!objectFilter->isAllValid())        // temp fix!!!
//            return;
    if (!layoutChanged)
        return;

    QFile savedLayoutFile(savedLayout);
    if (savedLayoutFile.open(QIODevice::WriteOnly))
    {
        QJsonObject layoutObject;
        write(layoutObject);
        QJsonDocument savedLayoutDoc(layoutObject);
        savedLayoutFile.write(savedLayoutDoc.toJson());
    }
    emit layoutSaveAttempted();
    qDebug() << "emit layoutSaveAttempted";
}
//! [5]

//! [6]
void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton)
        return;

    DiagramItem *item;
    switch (myMode) {
        case InsertItem:
            item = new DiagramItem(myItemType, "", myItemMenu);
            item->setBrush(myItemColor);
            addItem(item);
            item->setPos(mouseEvent->scenePos());
            emit itemInserted(item);
            break;
//! [6] //! [7]
        case InsertLine:
            line = new QGraphicsLineItem(QLineF(mouseEvent->scenePos(),
                                        mouseEvent->scenePos()));
            line->setPen(QPen(myLineColor, 2));
            addItem(line);
            break;
//! [7] //! [8]
        case InsertText:
            textItem = new DiagramTextItem();
            textItem->setFont(myFont);
            textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
            textItem->setZValue(1000.0);
            connect(textItem, SIGNAL(lostFocus(DiagramTextItem*)),
                    this, SLOT(editorLostFocus(DiagramTextItem*)));
            connect(textItem, SIGNAL(selectedChange(QGraphicsItem*)),
                    this, SIGNAL(itemSelected(QGraphicsItem*)));
            addItem(textItem);
            textItem->setDefaultTextColor(myTextColor);
            textItem->setPos(mouseEvent->scenePos());
            emit textInserted(textItem);
//! [8] //! [9]
    default:
        ;
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}
//! [9]

//! [10]
void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (myMode == InsertLine && line != 0) {
        QLineF newLine(line->line().p1(), mouseEvent->scenePos());
        line->setLine(newLine);
    } else if (myMode == MoveItem) {
        QGraphicsScene::mouseMoveEvent(mouseEvent);
    }
}
//! [10]

//! [11]
void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (line != 0 && myMode == InsertLine) {
        QList<QGraphicsItem *> startItems = items(line->line().p1());
        if (startItems.count() && startItems.first() == line)
            startItems.removeFirst();
        QList<QGraphicsItem *> endItems = items(line->line().p2());
        if (endItems.count() && endItems.first() == line)
            endItems.removeFirst();

        removeItem(line);
        delete line;
//! [11] //! [12]

        if (startItems.count() > 0 && endItems.count() > 0 &&
            startItems.first()->type() == DiagramItem::Type &&
            endItems.first()->type() == DiagramItem::Type
            // && startItems.first() != endItems.first() // prevents selfloops
                ) {
            DiagramItem *startItem = qgraphicsitem_cast<DiagramItem *>(startItems.first());
            DiagramItem *endItem = qgraphicsitem_cast<DiagramItem *>(endItems.first());
            Arrow *arrow = new Arrow();
            arrow->setColor(myLineColor);
            startItem->addArrow(arrow);
            if (startItem != endItem)
                endItem->addArrow(arrow);
            arrow->setZValue(-1000.0);
            addItem(arrow);
            arrow->updatePosition();
        }
    }
//! [12] //! [13]
    line = 0;
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}
#endif
void DiagramScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton)
        return;
    foreach (QGraphicsItem *item, QGraphicsScene::items(mouseEvent->scenePos()))
    {
        CanvasItem *canvasItem = qgraphicsitem_cast<CanvasItem*>(item);
        if (canvasItem)
        {
            selectedObject = itemHash.key(canvasItem);
            emit objectSelected(selectedObject);
            break;
        }
    }
    Canvas::mouseDoubleClickEvent(mouseEvent);
}

void DiagramScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *mouseEvent)
{
    selectedObject="";
    if (QGraphicsScene::items(mouseEvent->scenePos()).size()> 0)
    {
        CanvasItem *item = qgraphicsitem_cast<CanvasItem*>(QGraphicsScene::items(mouseEvent->scenePos()).at(0));
        selectedObject = itemHash.key(item);
    }
    if (selectedObject.isEmpty())
        return;

    QMenu menu;
    enableObserverAction = menu.addAction("Enable default observer");
    connect(enableObserverAction,SIGNAL(triggered()),this,SLOT(enableObserverClicked()));
    disableObserverAction = menu.addAction("Disable default observer");
    connect(disableObserverAction,SIGNAL(triggered()),this,SLOT(disableObserverClicked()));
    if (connections.contains(selectedObject))
    {
        lesionAction = menu.addAction("Lesion");
        connect(lesionAction,SIGNAL(triggered()),this,SLOT(lesionClicked()));
    }
    menu.exec(mouseEvent->screenPos());

    //    if (menu.exec(mouseEvent->screenPos())==enableObserverAction)
//        enableObserver(name);
//    else if (menu.exec(mouseEvent->screenPos())==disableObserverAction)
//        disableObserver(name);
//    else if (menu.exec(mouseEvent->screenPos())==lesionAction)
//        lesion(name);

}

void DiagramScene::enableObserverClicked()
{
    if(selectedObject.isEmpty())
        return;
    QString cmd = "(" + selectedObject + " default_observer) enable ";
    SessionManager::instance()->runCmd(cmd);

}

void DiagramScene::disableObserverClicked()
{
    if(selectedObject.isEmpty())
        return;
    QString cmd = "(" + selectedObject + " default_observer) disable ";
    SessionManager::instance()->runCmd(cmd);
}

void DiagramScene::lesionClicked()
{
    if(selectedObject.isEmpty())
        return;
    QString cmd = selectedObject + " lesion";
    SessionManager::instance()->runCmd(cmd);
}


void DiagramScene::positionObject(QString name, QString type, QDomDocument *domDoc)
{
    // layers are placed on the scene before arrows
    Q_UNUSED(domDoc)
//    if (type == "layer")
      if (type == boxType)
    {
//        DiagramItem *diagramItem = new DiagramItem(DiagramItem::Layer, name, myItemMenu);
//        diagramItem->setBrush(myItemColor);
        RectangleShape *box = new RectangleShape();
        // temporarily define dimensions here
        int boxWidth = 150;
        int boxHeight = 50;
        box->setPosAndSize(defaultPosition, QSizeF(boxWidth,boxHeight));
        box->setLabel(name);
        addItem(box);
//        currentPosition += itemOffset;
//        diagramItem->setPos(currentPosition);
        itemHash.insert(name,box);
    }
}

void DiagramScene::removeObject(QString name)
{
    CanvasItem* item = itemHash.value(name);
    if (!item)
        return;

    setSelection(QList<CanvasItem*>{item});
    deleteSelection();
    delete item;
    itemHash.remove(name);


//    if (item->type() == Arrow::Type)
//    {
//        removeItem(item);
//        Arrow *arrow = qgraphicsitem_cast<Arrow *>(item);
//        if (arrow->getStartItem())
//            arrow->getStartItem()->removeArrow(arrow);
//        if (arrow->getEndItem())
//            arrow->getEndItem()->removeArrow(arrow);
//        delete arrow;
//        itemHash.remove(name);
//    }
//    else if (item->type() == DiagramItem::Type)
//    {
//        DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem *>(item);
////        qgraphicsitem_cast<DiagramItem *>(item)->removeArrows();
//        removeItem(item);
//        delete diagramItem;
//        itemHash.remove(name);
//    }
}

void DiagramScene::renderObject(QDomDocument *domDoc)
{
    // wait until all descriptions of recently_* objects have arrived
    renderList.append(domDoc);
//    if (objectFilter->isAllValid()) // testing! to handle problem with missing conversion
        render();
}

//QString fixName (QString name);
//QString fixName (QString name)
//{
//    name = name.replace( " ", "_" );
//    name = name.replace( "(", "" );
//    name = name.replace( ")", "" );
//    return (name);
//}

void DiagramScene::render()
{
    // layers don't need any rendering and they are already in itemHash
    // connections need rendering since they need their source and dest layers, if present,
    // and are not in itemHash
//    QFile file(boxType+QString("connections.txt"));
//    file.open(QIODevice::Append | QIODevice::Text);
//    QTextStream out( &file );

    foreach(QDomDocument* domDoc, renderList)
    {
        if (!domDoc)
            continue;
//        if (AsLazyNutObject(*domDoc).type() == "connection")
            if (AsLazyNutObject(*domDoc).type() == arrowType)
        {
            QString name = AsLazyNutObject(*domDoc).name();
            qDebug() << "cycling through render:" << name;

            RectangleShape *startItem = qgraphicsitem_cast<RectangleShape *>
                    (itemHash.value(AsLazyNutObject(*domDoc)["Source"]()));
            RectangleShape *endItem = qgraphicsitem_cast<RectangleShape *>
                    (itemHash.value(AsLazyNutObject(*domDoc)["Target"]()));
            Connector *arrow;
            if (itemHash.contains(name))
                arrow = qgraphicsitem_cast<Connector*>(itemHash.value(name));
            else
//            {
                arrow = new Connector();
//                addItem(arrow);
//                itemHash.insert(name,arrow);
//            }
//            arrow->setStartItem(startItem);
//            arrow->setEndItem(endItem);
//            out << fixName(startItem->name()) << "-->"
//                     << fixName(endItem->name()) << ";\n";
            if (!startItem && !endItem)
            {
                arrow->setNewEndpoint(dunnart::SRCPT, defaultPosition, nullptr);
                arrow->setNewEndpoint(dunnart::DSTPT, defaultPosition - QPointF(0, 50), nullptr);
//                currentPosition += arrowOffset;
//                arrow->setArrowStart(currentPosition);
            }
            else if (!startItem)
            {
                arrow->setNewEndpoint(dunnart::DSTPT, endItem->centrePos(), endItem, dunnart::CENTRE_CONNECTION_PIN);
                arrow->setNewEndpoint(dunnart::SRCPT, endItem->centrePos() + QPointF(0,endItem->height()/2 + 50), nullptr);
            }
            else if (!endItem)
            {
                arrow->setNewEndpoint(dunnart::SRCPT, startItem->centrePos(), startItem, dunnart::CENTRE_CONNECTION_PIN);
                arrow->setNewEndpoint(dunnart::DSTPT, startItem->centrePos() - QPointF(0,endItem->height()/2 + 50), nullptr);
            }
            else
                arrow->initWithConnection(startItem, endItem);

            arrow->setDirected(true);

            if (!itemHash.contains(name))
            {
                addItem(arrow);
                itemHash.insert(name,arrow);
            }
//            if (startItem && startItem == endItem)
//                arrow->setArrowType(Arrow::SelfLoop);
//            else
//                arrow->setArrowType(Arrow::Line);
//            arrow->setColor(myLineColor);
//            if (startItem && !startItem->arrowList().contains(arrow))
//                startItem->addArrow(arrow);
//            if (endItem && !endItem->arrowList().contains(arrow))
//                endItem->addArrow(arrow);
//            if (startItem && endItem)
//                arrow->setZValue(-1000.0);
//            else
//                arrow->setZValue(1000.0);
//            arrow->updatePosition();
        }
    }
    renderList.clear();
    layout()->initialise();
    updateConnectorsForLayout();

    //    file.close();
}


//! [13]

//! [14]
#if 0
bool DiagramScene::isItemChange(int type)
{
    foreach (QGraphicsItem *item, selectedItems()) {
        if (item->type() == type)
            return true;
    }
    return false;
}
#endif
//! [14]

void DiagramScene::syncToObjCatalogue()
{
    qDebug() << "Entered syncToObjCatalogue()";
    QString name;
    QStringList newConnections{};
    // display new layers, hold new connections in a list
    for (int row=0;row<objectFilter->rowCount();row++)
    {
        name = objectFilter->data(objectFilter->index(row,0)).toString();
        if ((objectFilter->data(objectFilter->index(row,1)).toString() == boxType) &&
            (!itemHash.contains(name)))
            positionObject(name, boxType, nullptr);
        else if ((objectFilter->data(objectFilter->index(row,1)).toString() == arrowType))
//            &&   (!itemHash.contains(name)))
            newConnections << name;
    }

    //    display new connections
    foreach(QString name, newConnections)
    {
        connections.append(name);
//        qDebug() << "looking for" << name;
        QModelIndexList matchedIndexList = objectFilter->match(objectFilter->index(0,0),Qt::DisplayRole,name);
//        qDebug() << "matching row" << objectFilter->data(matchedIndexList.at(0));
        QVariant v = objectFilter->data(objectFilter->index(matchedIndexList.at(0).row(),3));
        if (v.canConvert<QDomDocument *>())
            renderList.append(v.value<QDomDocument *>());
        descriptionUpdater->requestDescription(name);
    }
    // remove destroyed objects
    foreach (QString name, itemHash.keys())
    {
        QModelIndexList matchedIndexList = objectFilter->match(objectFilter->index(0,0),Qt::DisplayRole,name);
        if (matchedIndexList.isEmpty())
            removeObject(name);
    }

//        DiagramItem *startItem = qgraphicsitem_cast<DiagramItem *>
//                        (itemHash->value(objectCatalogue->value(name)->getValue("Source")));
//            DiagramItem *endItem   = qgraphicsitem_cast<DiagramItem *>
//                        (itemHash->value(objectCatalogue->value(name)->getValue("Target")));
//            // arrows without start and end are not plotted in this version
//            if (!(startItem && endItem))
//                continue;
//            Arrow *arrow = new Arrow(name, startItem, endItem, Arrow::Excitatory);
//            arrow->setColor(myLineColor);
    //        startItem->addArrow(arrow);
    //        if (startItem != endItem)
    //            endItem->addArrow(arrow);
    //        arrow->setZValue(-1000.0);
    //        addItem(arrow);
    //        itemHash->insert(name,arrow);
    //    }
    //    // remove deleted objects
    //    // (see DesignWindow::deleteItem)
    //    foreach (QString name, itemHash->keys().toSet() - objectCatalogue->keys().toSet())
    //    {
    //        QGraphicsItem* item = itemHash->value(name);
    //        if (item->type() == Arrow::Type)
    //        {
    //            removeItem(item);
    //            Arrow *arrow = qgraphicsitem_cast<Arrow *>(item);
    //            arrow->getStartItem()->removeArrow(arrow);
    //            arrow->getEndItem()->removeArrow(arrow);
    //            delete item;
    //            itemHash->remove(name);
    //        }
    //    }
    //    foreach (QString name, itemHash->keys().toSet() - objectCatalogue->keys().toSet())
    //    {
    //        QGraphicsItem* item = itemHash->value(name);
    //        if (item->type() == DiagramItem::Type)
    //        {
    //            qgraphicsitem_cast<DiagramItem *>(item)->removeArrows();
    //            removeItem(item);
    //            delete item;
    //            itemHash->remove(name);
    //        }
    //    }
//    if (!layoutLoaded)
//    {
//        QFile savedLayoutFile(savedLayout);
//        if (savedLayoutFile.open(QIODevice::ReadOnly))
//        {
//            QByteArray savedLayoutData = savedLayoutFile.readAll();
//            QJsonDocument savedLayoutDoc(QJsonDocument::fromJson(savedLayoutData));
//            read(savedLayoutDoc.object());
//        }
//        //layoutLoaded = true;
//    }

//    render();
}
