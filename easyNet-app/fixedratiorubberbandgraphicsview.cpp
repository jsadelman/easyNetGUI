#include "fixedratiorubberbandgraphicsview.h"

#include <QMouseEvent>
#include <QRubberBand>
#include <QDebug>

FixedRatioRubberBandGraphicsView::FixedRatioRubberBandGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
}

void FixedRatioRubberBandGraphicsView::mousePressEvent(QMouseEvent *event)
{
    origin = event->pos();
    rubberBand->setGeometry(QRect(origin, QSize()));
    rubberBand->show();
}

void FixedRatioRubberBandGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QRect selectionRect = QRect(origin, event->pos()).normalized();
    int w = selectionRect.width() == 0 ? 1 : selectionRect.width();
    int h = selectionRect.height();
    if (h/w < geometry().height()/geometry().width())
    {
        selectionRect.setHeight(w * geometry().height()/geometry().width());
    }
    else
    {
        selectionRect.setWidth(h * geometry().width()/geometry().height());
    }
    rubberBand->setGeometry(selectionRect);
}

void FixedRatioRubberBandGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    rubberBand->hide();
    emit selectionRequested(rubberBand->geometry());
}
