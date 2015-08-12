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
//    void fitVisible();
    void loadLayout();
    void saveLayout();


private:
//    void fitVisible(bool computeBoundingRect);
    void read(const QJsonObject &json);
    void write(QJsonObject &json);
};

#endif // DIAGRAMVIEW_H
