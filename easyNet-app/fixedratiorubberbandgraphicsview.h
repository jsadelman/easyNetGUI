#ifndef FIXEDRATIORUBBERBANDGRAPHICSVIEW_H
#define FIXEDRATIORUBBERBANDGRAPHICSVIEW_H

#include <QGraphicsView>

class QRubberBand;

class FixedRatioRubberBandGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    FixedRatioRubberBandGraphicsView(QWidget *parent = Q_NULLPTR);

signals:
    void selectionRequested(QRect);

protected:
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    QPoint origin;
    QRubberBand *rubberBand;
};

#endif // FIXEDRATIORUBBERBANDGRAPHICSVIEW_H
