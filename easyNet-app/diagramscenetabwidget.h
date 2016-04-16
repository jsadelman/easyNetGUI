#ifndef DIAGRAMSCENETABWIDGET_H
#define DIAGRAMSCENETABWIDGET_H

#include <QTabWidget>

class DiagramScene;
class DiagramView;


class DiagramSceneTabWidget : public QTabWidget
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
    void zoomChanged();

private slots:
    void emitCurrentDiagramSceneChanged(DiagramScene* canvas);
};

#endif // DIAGRAMSCENETABWIDGET_H
