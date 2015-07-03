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


#include <QTextCursor>
#include <QGraphicsSceneMouseEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QMetaObject>
#include <QDomDocument>
#include <QDebug>

Q_DECLARE_METATYPE(QDomDocument*)

//! [0]
DiagramScene::DiagramScene(QMenu *itemMenu, ObjectCatalogue *objectCatalogue,
                           QString _boxType, QString _arrowType, QObject *parent)
    : objectCatalogue(objectCatalogue), QGraphicsScene(parent)
{
    boxType =  _boxType;
    arrowType =  _arrowType;
    myItemMenu = itemMenu;
    myMode = MoveItem;
    myItemType = DiagramItem::Layer;
    line = 0;
    textItem = 0;
    myItemColor = Qt::white;
    myTextColor = Qt::black;
    myLineColor = Qt::black;
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
        }
        //layoutLoaded = true;
    }

//    disconnect(descriptionUpdater, SIGNAL(descriptionUpdated(QDomDocument*)),
//                   this, SLOT(loadLayout()));
}

void DiagramScene::setSelected(QString name)
{
    if (itemHash.contains(name))
    {
        clearSelection();
        foreach (QGraphicsItem *item, items())
        {
            if (item == itemHash.value(name))
            {
                item->setSelected(true);
                return;
            }
        }
    }
}

void DiagramScene::savedLayoutToBeLoaded(QString _savedLayout)
{
    savedLayout = _savedLayout;
    layoutLoaded = !(QFileInfo(savedLayout).exists());
}

void DiagramScene::saveLayout()
{
//    if (!objectFilter->isAllValid())        // temp fix!!!
//            return;

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

void DiagramScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton)
        return;
    if (myMode == MoveItem)
    {
        if (items(mouseEvent->scenePos()).size()> 0)
        {
            QGraphicsItem *item = items(mouseEvent->scenePos()).at(0);
            QString name = itemHash.key(item);
//            if (!name.isEmpty())
                emit objectSelected(name);
        }
    }
    QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
}

void DiagramScene::positionObject(QString name, QString type, QDomDocument *domDoc)
{
    // layers are placed on the scene before arrows
    Q_UNUSED(domDoc)
//    if (type == "layer")
      if (type == boxType)
    {
        DiagramItem *diagramItem = new DiagramItem(DiagramItem::Layer, name, myItemMenu);
        diagramItem->setBrush(myItemColor);
        addItem(diagramItem);
        currentPosition += itemOffset;
        diagramItem->setPos(currentPosition);
        itemHash.insert(name,diagramItem);
    }
}

void DiagramScene::removeObject(QString name)
{
    QGraphicsItem* item = itemHash.value(name);
    if (item->type() == Arrow::Type)
    {
        removeItem(item);
        Arrow *arrow = qgraphicsitem_cast<Arrow *>(item);
        if (arrow->getStartItem())
            arrow->getStartItem()->removeArrow(arrow);
        if (arrow->getEndItem())
            arrow->getEndItem()->removeArrow(arrow);
        delete arrow;
        itemHash.remove(name);
    }
    else if (item->type() == DiagramItem::Type)
    {
        DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem *>(item);
//        qgraphicsitem_cast<DiagramItem *>(item)->removeArrows();
        removeItem(item);
        delete diagramItem;
        itemHash.remove(name);
    }
}

void DiagramScene::renderObject(QDomDocument *domDoc)
{
    // wait until all descriptions of recently_* objects have arrived
    renderList.append(domDoc);
//    if (objectFilter->isAllValid()) // testing! to handle problem with missing conversion
        render();
}

QString fixName (QString name);
QString fixName (QString name)
{
    name = name.replace( " ", "_" );
    name = name.replace( "(", "" );
    name = name.replace( ")", "" );
    return (name);
}

void DiagramScene::render()
{
    // layers don't need any rendering and they are already in itemHash
    // connections need rendering since they need their source and dest layers, if present,
    // and are not in itemHash
    QFile file(boxType+QString("connections.txt"));
    file.open(QIODevice::Append | QIODevice::Text);
    QTextStream out( &file );

    foreach(QDomDocument* domDoc, renderList)
    {
//        if (AsLazyNutObject(*domDoc).type() == "connection")
            if (AsLazyNutObject(*domDoc).type() == arrowType)
        {
            QString name = AsLazyNutObject(*domDoc).name();
            DiagramItem *startItem = qgraphicsitem_cast<DiagramItem *>
                    (itemHash.value(AsLazyNutObject(*domDoc)["Source"]()));
            DiagramItem *endItem = qgraphicsitem_cast<DiagramItem *>
                    (itemHash.value(AsLazyNutObject(*domDoc)["Target"]()));
            Arrow *arrow;
            if (itemHash.contains(name))
                arrow = qgraphicsitem_cast<Arrow*>(itemHash.value(name));
            else
            {
                arrow = new Arrow(name);
                addItem(arrow);
                itemHash.insert(name,arrow);
            }
            arrow->setStartItem(startItem);
            arrow->setEndItem(endItem);
            out << fixName(startItem->name()) << "-->"
                     << fixName(endItem->name()) << ";\n";
            if (!startItem && !endItem)
            {
                currentPosition += arrowOffset;
                arrow->setArrowStart(currentPosition);
            }
            if (startItem && startItem == endItem)
                arrow->setArrowType(Arrow::SelfLoop);
            else
                arrow->setArrowType(Arrow::Line);
            arrow->setColor(myLineColor);
            if (startItem && !startItem->arrowList().contains(arrow))
                startItem->addArrow(arrow);
            if (endItem && !endItem->arrowList().contains(arrow))
                endItem->addArrow(arrow);
            if (startItem && endItem)
                arrow->setZValue(-1000.0);
            else
                arrow->setZValue(1000.0);
            arrow->updatePosition();
        }
    }
    renderList.clear();
    file.close();
}

//! [13]

//! [14]
bool DiagramScene::isItemChange(int type)
{
    foreach (QGraphicsItem *item, selectedItems()) {
        if (item->type() == type)
            return true;
    }
    return false;
}
//! [14]

void DiagramScene::syncToObjCatalogue()
{
    QString name;
    QStringList newConnections{};
    // display new layers, hold new connections in a list
    for (int row=0;row<objectFilter->columnCount();row++)
    {
        name = objectFilter->data(objectFilter->index(row,0)).toString();
        if ((objectFilter->data(objectFilter->index(row,1)).toString() == boxType) &&
            (!itemHash.contains(name)))
            positionObject(name, boxType, nullptr);
        else if ((objectFilter->data(objectFilter->index(row,1)).toString() == arrowType) &&
                (!itemHash.contains(name)))
            newConnections << name;
    }

    //    display new connections
    QModelIndexList tmp;
    QDomDocument* domDoc;
    foreach(QString name, newConnections)
    {
        tmp.clear();
        qDebug() << "looking for" << name;
        tmp << objectFilter->match(objectFilter->index(0,0),Qt::DisplayRole,name);
        qDebug() << "matching row" << objectFilter->data(tmp.at(0));
        QVariant v = objectFilter->data(objectFilter->index((tmp.at(0)).row(),2));
        if (v.canConvert<QDomDocument *>())
            renderList.append(v.value<QDomDocument *>());
        descriptionUpdater->requestDescription(name);

//        qDebug() << "domDoc" << domDoc;
//        renderList.append(domDoc);
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
    if (!layoutLoaded)
    {
        QFile savedLayoutFile(savedLayout);
        if (savedLayoutFile.open(QIODevice::ReadOnly))
        {
            QByteArray savedLayoutData = savedLayoutFile.readAll();
            QJsonDocument savedLayoutDoc(QJsonDocument::fromJson(savedLayoutData));
            read(savedLayoutDoc.object());
        }
        //layoutLoaded = true;
    }

    render();
}
