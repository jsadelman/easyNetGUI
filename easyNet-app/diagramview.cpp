#include "diagramview.h"
#include "canvasview.h"

#include <QScrollBar>
#include <QDebug>


DiagramView::DiagramView(DiagramScene *scene) : CanvasView(scene)
{
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
    QRectF sceneRect = computeBoundingRect ? canvas()->itemsBoundingRect() :
                                             canvas()->sceneRect();
    qreal viewBorder = 50;
    qreal sceneHeight = sceneRect.height() + viewBorder;
    qreal sceneWidth = sceneRect.width() + viewBorder;
    QRectF viewRect = geometry();
    qreal viewHeight = viewRect.height() - horizontalScrollBar()->height();
    qreal viewWidth = viewRect.width() - verticalScrollBar()->width();
    qreal newScale = qMin(viewHeight/sceneHeight, viewWidth/sceneWidth);
    newScale = qMin(1.0, newScale);
    scale(newScale, newScale);
    centerOn(sceneRect.center());
    emit zoomChanged();
}
