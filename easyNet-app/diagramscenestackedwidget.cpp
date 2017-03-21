#include "DiagramSceneStackedWidget.h"
#include "diagramscene.h"
#include "diagramview.h"

#include <QUndoGroup>
#include <QDebug>

DiagramSceneStackedWidget::DiagramSceneStackedWidget(QMainWindow *window)
    : QStackedWidget(window)//,m_action_delete(new QAction()),
      //m_action_redo(new QAction),m_action_undo(new QAction)
{
}

int DiagramSceneStackedWidget::newDiagramScene(QString title, QString boxType, QString arrowType)
{
    DiagramScene *scene = new DiagramScene(title,boxType, arrowType);
    connect(scene, &DiagramScene::initArrangement, [=]()
    {
//       if (scene != emptyScene)
           emit initArrangement(scene);
    });
    DiagramView *view = new DiagramView(scene);
    //m_undo_group->addStack(scene->undoStack());
    connect(view, SIGNAL(zoomChanged()), this, SIGNAL(zoomChanged()));

    int i=addWidget(view);

    modelIndex[title]=i;

    return i;
}

DiagramScene *DiagramSceneStackedWidget::currentDiagramScene()
{
    if(held.contains(currentIndex()))
    {
        return qobject_cast<DiagramScene*>(qobject_cast<DiagramView*>(held[currentIndex()])->scene());
    }
    return qobject_cast<DiagramScene*>(diagramSceneAt(currentIndex()));
}

DiagramView *DiagramSceneStackedWidget::currentDiagramView()
{
    if(held.contains(currentIndex()))
    {
        return qobject_cast<DiagramView*>(held[currentIndex()]);
    }
    return qobject_cast<DiagramView*>(currentWidget());
}

DiagramScene *DiagramSceneStackedWidget::diagramSceneAt(int index)
{
    if(held.contains(index))
    {
        return qobject_cast<DiagramScene*>(qobject_cast<DiagramView*>(held[index])->scene());
    }
    return qobject_cast<DiagramScene*>(diagramViewAt(index)->scene());
}

DiagramView *DiagramSceneStackedWidget::diagramViewAt(int index)
{
    if(held.contains(index))
    {
        return qobject_cast<DiagramView*>(held[index]);
    }
    return qobject_cast<DiagramView*>(widget(index));
}

void DiagramSceneStackedWidget::poke()
{
    for(auto x:held) x->update();
}

/*void DiagramSceneStackedWidget::useFake(int idx, bool yn)
{
    if (yn && !held.contains(idx))
    {
        held.insert(idx,widget(idx));
        insertWidget(idx,new DiagramView(emptyScene)); // ,lab);
        removeWidget(widget(idx+1));
        setCurrentIndex(idx);
    }
    else if (!yn && held.contains(idx))
    {
        qDebug()<<"returning";
        QWidget* todel=widget(idx);
        QWidget* returned=held[idx];
        insertWidget(idx,returned); // ',lab);
        removeWidget(widget(idx+1));
        delete todel;
        setCurrentIndex(idx);
        held.remove(idx);
    }
}
*/
void DiagramSceneStackedWidget::emitCurrentDiagramSceneChanged(QGraphicsScene *canvas)
{
    DiagramScene* scene = qobject_cast<DiagramScene*>(canvas);
    if (scene)
        emit currentDiagramSceneChanged(scene);
}
