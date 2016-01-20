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
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "sessionmanager.h"
#include "objectcache.h"

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
    : m_boxType(box_type), m_arrowType(arrow_type), awake(false), Canvas()
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

    setNewModelLoaded(false);

    setProperty("structuralEditingDisabled", true);
    setProperty("idealEdgeLengthModifier", 1.0);
    setProperty("preventOverlaps", true);

    // box parameters
    boxLongNameToDisplayIntact = "longnameXX";
    boxWidthMarginProportionToLongestLabel = 0.1;
    boxWidthOverHeight = 1.618;
    // compute box width and use it to set shapeNonOverlapPadding
    QFontMetrics fm(canvasFont());
    qreal boxWidth = (1.0 + 2.0 * boxWidthMarginProportionToLongestLabel) * fm.width(boxLongNameToDisplayIntact);
    int shapeNonOverlapPadding = boxWidth * 0.4; // just an estimate
    qreal idealConnectorLength = boxWidth * 2.0; // just an estimate
    jitter = boxWidth * 0.3; // just an estimate

    setProperty("idealConnectorLength", idealConnectorLength);
    setProperty("shapeNonOverlapPadding", shapeNonOverlapPadding);

    defaultPosition = QPointF(0,0); //  150);
    currentPosition = defaultPosition;
    itemOffset = QPointF(0,50) ; // 150);
    arrowOffset = QPointF(50,0);

    boxFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    boxFilter->setType(m_boxType);



    arrowFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    arrowFilter->setType(m_arrowType);

    boxDescriptionUpdater = new ObjectUpdater(this);
    boxDescriptionUpdater->setProxyModel(boxFilter);

    arrowDescriptionUpdater = new ObjectUpdater(this);
    arrowDescriptionUpdater->setProxyModel(arrowFilter);

    connect(boxDescriptionUpdater, SIGNAL(objectUpdated(QDomDocument*, QString)),
            this, SLOT(renderObject(QDomDocument*)));

    connect(arrowDescriptionUpdater, SIGNAL(objectUpdated(QDomDocument*, QString)),
            this, SLOT(renderObject(QDomDocument*)));
    connect(m_animation_group, SIGNAL(finished()), this, SIGNAL(animationFinished()));

    // default state is wake up
    wakeUp();
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
    qreal boxWidth = json["boxWidth"].toDouble();
    qDebug() << "DiagramScene::read boxWidth" << boxWidth;
    QJsonArray itemArray = json["diagramItems"].toArray();
    for (int itemIndex = 0; itemIndex < itemArray.size(); ++itemIndex)
    {
        QJsonObject itemObject = itemArray[itemIndex].toObject();
        QString name = itemObject["name"].toString();
        if (itemHash.contains(name))
        {
            Box * box = qobject_cast<Box*>(itemHash.value(name));
            if (box)
                box->read(itemObject, boxWidth);
        }
    }
}

void DiagramScene::write(QJsonObject &json)
{
    QJsonArray itemArray;
    qreal boxWidth=0;
    foreach (Box * box, boxes())
    {
            QJsonObject itemObject;
            box->write(itemObject);
            itemArray.append(itemObject);
            if (boxWidth == 0)
                boxWidth = box->autoWidth();
    }
    json["diagramItems"] = itemArray;
    json["boxWidth"] = boxWidth;
}

void DiagramScene::setBaseName(QString baseName)
{
    m_baseName = baseName;
    m_layoutFile = m_baseName.append(QString(".%1.json").arg(m_boxType));
}

bool DiagramScene::validForAlignment(QList<dunnart::CanvasItem *> items)
{
    // a list of selected items should contain at least one ShapeObj,
    // otherwise Dunnart crashes.
    foreach(dunnart::CanvasItem *item, items)
    {
        if (qobject_cast<ShapeObj*>(item))
            return true;
    }
    return false;
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
//    connect(descriptionUpdater, SIGNAL(objectUpdated(QDomDocument*)),
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

void DiagramScene::wakeUp()
{
    if (!awake)
    {
        connect(boxFilter, SIGNAL(objectCreated(QString, QString, QString, QDomDocument*)),
                this, SLOT(positionObject(QString, QString, QString, QDomDocument*)));
        connect(boxFilter, SIGNAL(objectDestroyed(QString)),
                this, SLOT(removeObject(QString)));
        connect(arrowFilter, SIGNAL(objectDestroyed(QString)),
                this, SLOT(removeObject(QString)));
        boxDescriptionUpdater->wakeUpUpdate();
        arrowDescriptionUpdater->wakeUpUpdate();
        syncToObjCatalogue();
        awake = true;
    }
}

void DiagramScene::goToSleep()
{
    if (awake)
    {
        disconnect(boxFilter, SIGNAL(objectCreated(QString, QString, QString, QDomDocument*)),
                   this, SLOT(positionObject(QString, QString, QString, QDomDocument*)));
        disconnect(boxFilter, SIGNAL(objectDestroyed(QString)),
                   this, SLOT(removeObject(QString)));
        disconnect(arrowFilter, SIGNAL(objectDestroyed(QString)),
                   this, SLOT(removeObject(QString)));
        boxDescriptionUpdater->goToSleep();
        arrowDescriptionUpdater->goToSleep();
        awake = false;
    }
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

void DiagramScene::positionObject(QString name, QString type, QString subtype, QDomDocument *domDoc)
{
    // layers are placed on the scene before arrows
    Q_UNUSED(domDoc)
    Q_UNUSED(subtype)
      if (type == m_boxType)
    {
        Box *box = new Box();
        connect(box, SIGNAL(createNewRPlot(QString,QString,QMap<QString, QString>,int)),
                this, SIGNAL(createNewRPlot(QString,QString,QMap<QString,QString>,int)));

        addItem(box);
        box->setName(name); // set name before type, otherwise defaultDataframesFilter won't get properly set for layers
        box->setLazyNutType(m_boxType);
        connect(box, SIGNAL(plotDestroyed(QString)), this, SIGNAL(plotDestroyed(QString)));




        // temporarily define dimensions here

        box->setProperty("position", defaultPosition);
        box->setProperty("longNameToDisplayIntact", boxLongNameToDisplayIntact);
        box->setProperty("widthMarginProportionToLongestLabel", boxWidthMarginProportionToLongestLabel);
        box->setProperty("widthOverHeight", boxWidthOverHeight);
//        box->setLabelPointSize(14);
        box->autoSize();
        box->setLabel(name);
        box->setToolTip(name);
//        if (m_boxType == "representation")
//            box->setFillColour(QColor("azure"));

        itemHash.insert(name,box);
    }
}

void DiagramScene::removeObject(QString name)
{
    CanvasItem* item = itemHash.value(name);
    if (!item)
        return;

    setSelection(QList<CanvasItem*>{item});
    setProperty("structuralEditingDisabled", false);
    deleteSelection();
    setProperty("structuralEditingDisabled", true);
    delete item;
    itemHash.remove(name);


}

void DiagramScene::renderObject(QDomDocument *domDoc)
{
    // wait until all descriptions of recently_* objects have arrived
    renderList.append(domDoc);
    if (boxFilter->isAllValid() && arrowFilter->isAllValid())
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
        if (AsLazyNutObject(*domDoc).type() == m_boxType)
        {
            QString name = AsLazyNutObject(*domDoc).name();
            QMap <QString, QString> ports;
            foreach (QString label, XMLelement(*domDoc)["Ports"].listLabels())
                ports[label] = XMLelement(*domDoc)["Ports"][label]();

            static_cast<Box*>(itemHash.value(name))->setPorts(ports);
        }
        else if (AsLazyNutObject(*domDoc).type() == m_arrowType)
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
    if (newModelLoaded())
    {
        setNewModelLoaded(false);
        emit initArrangement();
    }

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
    QString name;
    QString subtype;
    // display new layers, hold new connections in a list
    for (int row=0;row<boxFilter->rowCount();row++)
    {
        name = boxFilter->data(boxFilter->index(row,ObjectCache::NameCol)).toString();
        subtype = boxFilter->data(boxFilter->index(row,ObjectCache::SubtypeCol)).toString();
        if (!itemHash.contains(name))
        {
            positionObject(name, m_boxType, subtype, nullptr);
            QVariant v = boxFilter->data(boxFilter->index(row, ObjectCache::DomDocCol));
            if (v.canConvert<QDomDocument *>())
                renderList.append(v.value<QDomDocument *>());
            boxDescriptionUpdater->requestObject(name);
        }
//        else if ((boxFilter->data(boxFilter->index(row,1)).toString() == m_arrowType))
//            &&   (!itemHash.contains(name)))
//            newConnections << name;
    }

    //    display new connections
    for (int row=0;row<arrowFilter->rowCount();row++)
    {
        name = arrowFilter->data(arrowFilter->index(row,0)).toString();
        if (!itemHash.contains(name))
        {
            connections.append(name); // OBSOLETE
            QVariant v = arrowFilter->data(arrowFilter->index(row, ObjectCache::DomDocCol));
            if (v.canConvert<QDomDocument *>())
                renderList.append(v.value<QDomDocument *>());
            arrowDescriptionUpdater->requestObject(name);
        }
    }

   # if 0
    foreach(QString name, newConnections)
    {
        connections.append(name);
        QModelIndexList matchedIndexList = objectFilter->match(objectFilter->index(0,0),Qt::DisplayRole,name);
        QVariant v = objectFilter->data(objectFilter->index(matchedIndexList.at(0).row(),3));
        if (v.canConvert<QDomDocument *>())
            renderList.append(v.value<QDomDocument *>());
        arrowDescriptionUpdater->requestObject(name);
    }
#endif
    // remove destroyed objects
    foreach (QString name, itemHash.keys())
    {
        QModelIndexList matchedIndexList = boxFilter->match(boxFilter->index(0,0),Qt::DisplayRole,name);
        matchedIndexList.append(arrowFilter->match(arrowFilter->index(0,0),Qt::DisplayRole,name));
        if (matchedIndexList.isEmpty())
            removeObject(name);
    }

}
