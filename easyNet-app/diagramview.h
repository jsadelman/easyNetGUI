#ifndef DIAGRAMVIEW_H
#define DIAGRAMVIEW_H

#include "libdunnartcanvas/canvasview.h"
#include "diagramscene.h"



class DiagramView : public dunnart::CanvasView
{
    Q_OBJECT
public:
    explicit DiagramView(DiagramScene *scene);

public slots:
    void fitVisible();

private:
    void fitVisible(bool computeBoundingRect);
};

#endif // DIAGRAMVIEW_H
