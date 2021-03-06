#include "diagramview.h"

#include "diagramscene.h"

#include <QScrollBar>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRectF>

DiagramView::DiagramView(DiagramScene *scene) : QGraphicsView(scene)
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(QGraphicsView::RubberBandDrag);

}

void DiagramView::fitVisible()
{
//    DiagramScene *scene = qobject_cast<DiagramScene*>(sender());
//    fitVisible(scene != NULL);
    fitVisible(true);
}

void DiagramView::fitVisible(bool computeBoundingRect)
{
    resetTransform();
//    scene()->removeItem(canvas()->getZeb());
    QRectF sceneRect = computeBoundingRect ? canvas()->itemsBoundingRect() :
                                             canvas()->sceneRect();
//    scene()->addItem(canvas()->getZeb());
    qreal viewBorder = 100;
    qreal sceneHeight = sceneRect.height() + viewBorder;
    qreal sceneWidth = sceneRect.width() + viewBorder;
    QRectF viewRect = geometry();
    qreal viewHeight = viewRect.height() - (horizontalScrollBar()->isVisible() ?  horizontalScrollBar()->height() : 0);
    qreal viewWidth = viewRect.width() - (verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0);
    qreal newScale = qMin(viewHeight/sceneHeight, viewWidth/sceneWidth);
//    newScale = qMin(1.0, newScale);
    scale(newScale, newScale);
    centerOn(sceneRect.center());
    emit zoomChanged();
}

void DiagramView::read(const QJsonObject &json)
{
    qobject_cast<DiagramScene*>(canvas())->read(json);
    canvas()->updateConnectorsForLayout();
# if 0
    qreal zoomScale = json["zoomScale"].toDouble();
    qreal zoomDx = json["zoomDx"].toDouble();
    qreal zoomDy = json["zoomDy"].toDouble();
    resetTransform();
    translate(zoomDx, zoomDy);
    scale(zoomScale, zoomScale);
    emit zoomChanged();
#endif
}

void DiagramView::write(QJsonObject &json)
{
    qobject_cast<DiagramScene*>(canvas())->write(json);
#if 0
    json["zoomScale"] = transform().m11();
    json["zoomDx"] = transform().m31();
    json["zoomDy"] = transform().m32();
#endif
}


void DiagramView::loadLayout()
{
//    if (!objectFilter->isAllValid())
//        return;
        QString layoutFile = qobject_cast<DiagramScene*>(canvas())->layoutFile();
        QFile savedLayoutFile(layoutFile);
        if (savedLayoutFile.open(QIODevice::ReadOnly))
        {
            QByteArray savedLayoutData = savedLayoutFile.readAll();
            QJsonDocument savedLayoutDoc(QJsonDocument::fromJson(savedLayoutData));
            read(savedLayoutDoc.object());
        }
}

void DiagramView::saveLayout()
{
//    if (!objectFilter->isAllValid())        // temp fix!!!
//            return;

    QString layoutFile = qobject_cast<DiagramScene*>(canvas())->layoutFile();
    QFile savedLayoutFile(layoutFile);
    if (savedLayoutFile.open(QIODevice::WriteOnly))
    {
        QJsonObject layoutObject;
        write(layoutObject);
        QJsonDocument savedLayoutDoc(layoutObject);
        savedLayoutFile.write(savedLayoutDoc.toJson());
    }
}

