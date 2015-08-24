#ifndef DIAGRAMSCENETABWIDGET_H
#define DIAGRAMSCENETABWIDGET_H

#include "libdunnartcanvas/canvastabwidget.h"

class DiagramScene;
class DiagramView;


class DiagramSceneTabWidget : public dunnart::CanvasTabWidget
{
    Q_OBJECT
public:
    explicit DiagramSceneTabWidget(QMainWindow *window);
    int newDiagramScene(QString title, QString boxType, QString arrowType);
    DiagramScene *currentDiagramScene();
    DiagramView *currentDiagramView();
    DiagramScene *diagramSceneAt(int index);
    DiagramView *diagramViewAt(int index);

signals:
    void currentDiagramSceneChanged(DiagramScene *scene);
    void initArrangement();

private slots:
    void emitCurrentDiagramSceneChanged(dunnart::Canvas* canvas);
};

#endif // DIAGRAMSCENETABWIDGET_H
