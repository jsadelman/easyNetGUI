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

#include "box.h"
#include "arrow.h"
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
DiagramScene::DiagramScene(QString box_type, QString arrow_type)
    : m_boxType(box_type), m_arrowType(arrow_type), Canvas()
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

    setProperty("structuralEditingDisabled", true);
    setProperty("idealEdgeLengthModifier", 1.0);
    setProperty("preventOverlaps", true);

    // box parameters
    boxLongNameToDisplayIntact = "longnameof_level";
    boxWidthMarginProportionToLongestLabel = 0.1;
    boxWidthOverHeight = 1.618;
    // compute box width and use it to set shapeNonOverlapPadding
    QFontMetrics fm(canvasFont());
    qreal boxWidth = (1.0 + 2.0 * boxWidthMarginProportionToLongestLabel) * fm.width(boxLongNameToDisplayIntact);
    int shapeNonOverlapPadding = boxWidth * 0.4; // just an estimate
    qreal idealConnectorLength = boxWidth * 2.0;
    jitter = boxWidth * 0.3; // just an estimate

    setProperty("idealConnectorLength", idealConnectorLength);
    setProperty("shapeNonOverlapPadding", shapeNonOverlapPadding);

    defaultPosition = QPointF(0,0); //  150);
    currentPosition = defaultPosition;
    itemOffset = QPointF(0,50) ; // 150);
    arrowOffset = QPointF(50,0);

    objectFilter = new ObjectCatalogueFilter(this);
    objectFilter->setTypeList(QStringList({m_boxType, m_arrowType}));
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
    connect(m_animation_group, SIGNAL(finished()), this, SIGNAL(animationFinished()));
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

QList<Box *> DiagramScene::boxes()
{
    QList<Box *> result;
    QListIterator<CanvasItem*> it(items());
    while(it.hasNext())
    {
        Box *box = qobject_cast<Box*>(it.next());
        if (box)
            result.append(box);
    }
    return result;
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
            Box * box = qobject_cast<Box*>(itemHash.value(name));
            if (box)
                box->read(itemObject);
        }
    }
}

void DiagramScene::write(QJsonObject &json)
{
    QJsonArray itemArray;
    foreach (Box * box, boxes())
    {
            QJsonObject itemObject;
            box->write(itemObject);
            itemArray.append(itemObject);
    }
    json["diagramItems"] = itemArray;
}

void DiagramScene::setBaseName(QString baseName)
{
    m_baseName = baseName;
    m_layoutFile = m_baseName.append(QString(".%1.json").arg(m_boxType));
}
//! [4]

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
#endif


void DiagramScene::setSelected(QString name)
{
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
    int maxCCsize = cc.length() == 0 ? 0 :
            (*std::max_element(cc.begin(), cc.end(),
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
                              QPointF((double)(qrand() % 10 - 5)*jitter, (double)(qrand() % 10 - 5)*jitter));
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

void DiagramScene::positionObject(QString name, QString type, QDomDocument *domDoc)
{
    // layers are placed on the scene before arrows
    Q_UNUSED(domDoc)
//    if (type == "layer")
      if (type == m_boxType)
    {
//        DiagramItem *diagramItem = new DiagramItem(DiagramItem::Layer, name, myItemMenu);
//        diagramItem->setBrush(myItemColor);
        Box *box = new Box();
        connect(box, SIGNAL(createNewPlotOfType(QString,QString,QMap<QString, QString>)),
                this, SIGNAL(createNewPlotOfType(QString,QString,QMap<QString,QString>)));

        addItem(box);
        box->setName(name);
        box->setLazyNutType(m_boxType);
        // temporarily define dimensions here

//        int boxHeight = 80;
//        int boxWidth = qCeil((qreal)boxHeight * 1.618);
        box->setProperty("position", defaultPosition);
        box->setProperty("longNameToDisplayIntact", boxLongNameToDisplayIntact);
        box->setProperty("widthMarginProportionToLongestLabel", boxWidthMarginProportionToLongestLabel);
        box->setProperty("widthOverHeight", boxWidthOverHeight);
        box->autoSize();
//        box->setPosAndSize(defaultPosition, QSizeF(boxWidth,boxHeight));

        box->setLabel(name);
        box->setToolTip(name);
        if (m_boxType == "representation")
            box->setFillColour(QColor("azure"));

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


}

void DiagramScene::renderObject(QDomDocument *domDoc)
{
    // wait until all descriptions of recently_* objects have arrived
    renderList.append(domDoc);
//    if (objectFilter->isAllValid()) // testing! to handle problem with missing conversion
        render();
}


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
            if (AsLazyNutObject(*domDoc).type() == m_arrowType)
        {
            QString name = AsLazyNutObject(*domDoc).name();
            Box *startItem = qgraphicsitem_cast<Box *>
                    (itemHash.value(AsLazyNutObject(*domDoc)["Source"]()));
            Box *endItem = qgraphicsitem_cast<Box *>
                    (itemHash.value(AsLazyNutObject(*domDoc)["Target"]()));
            Arrow *arrow;
            if (itemHash.contains(name))
                arrow = qgraphicsitem_cast<Arrow*>(itemHash.value(name));
            else
            {
                arrow = new Arrow();
                arrow->setName(name);
                arrow->setLazyNutType(m_arrowType);
                arrow->setToolTip(name);
            }

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
                arrow->setNewEndpoint(dunnart::DSTPT, startItem->centrePos() - QPointF(0,startItem->height()/2 + 50), nullptr);
            }
            else
                arrow->initWithConnection(startItem, endItem);

            arrow->setDirected(true);

            if (!itemHash.contains(name))
            {
                addItem(arrow);
                itemHash.insert(name,arrow);
            }
        }
    }
    renderList.clear();
    layout()->initialise();
    updateConnectorsForLayout();

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
        if ((objectFilter->data(objectFilter->index(row,1)).toString() == m_boxType) &&
            (!itemHash.contains(name)))
            positionObject(name, m_boxType, nullptr);
        else if ((objectFilter->data(objectFilter->index(row,1)).toString() == m_arrowType))
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

}
