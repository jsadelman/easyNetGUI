#ifndef DIAGRAMVIEW_H
#define DIAGRAMVIEW_H

#include "diagramscene.h"

#include <QGraphicsView>

class DiagramView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DiagramView(DiagramScene *scene);

public slots:
    void fitVisible();
    void fitVisible(bool);
    void loadLayout();
    void saveLayout();
    void zoomChanged();
    DiagramScene* canvas(){return qobject_cast<DiagramScene*>(scene());}
private:
//    void fitVisible(bool computeBoundingRect);
    void read(const QJsonObject &json);
    void write(QJsonObject &json);
};

#endif // DIAGRAMVIEW_H
