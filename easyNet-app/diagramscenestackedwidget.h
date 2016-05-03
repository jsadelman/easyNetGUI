#ifndef DIAGRAMSCENESTACKEDWIDGET_H
#define DIAGRAMSCENESTACKEDWIDGET_H

#include <QStackedWidget>
#include <QMainWindow>
#include <QAction>
#include <QUndoGroup>

#include <QGraphicsScene>
class DiagramScene;
class DiagramView;


class DiagramSceneStackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit DiagramSceneStackedWidget(QMainWindow *window);
    int newDiagramScene(QString title, QString boxType, QString arrowType);
    DiagramScene *currentDiagramScene();
    DiagramView *currentDiagramView();
    DiagramScene *diagramSceneAt(int index);
    DiagramView *diagramViewAt(int index);
    DiagramScene *currentCanvas(){return currentDiagramScene();}
    DiagramView *currentCanvasView(){return currentDiagramView();}
    void useFake(int idx,bool yn);

    void poke();
signals:
    void currentDiagramSceneChanged(DiagramScene *scene);
    void initArrangement();
    void zoomChanged();

private slots:
    void emitCurrentDiagramSceneChanged(QGraphicsScene* canvas);
 private:
    QAction *m_action_delete,    *m_action_undo,    *m_action_redo;
    QUndoGroup *m_undo_group;
    QMap<int,QWidget*> held;
    DiagramScene*emptyScene;
};

#endif // DIAGRAMSCENESTACKEDWIDGET_H
