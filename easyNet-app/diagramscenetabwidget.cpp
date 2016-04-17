#include "diagramscenetabwidget.h"
#include "diagramscene.h"
#include "diagramview.h"

#include <QUndoGroup>
#include <QDebug>

DiagramSceneTabWidget::DiagramSceneTabWidget(QMainWindow *window)
    : QTabWidget(window)//,m_action_delete(new QAction()),
      //m_action_redo(new QAction),m_action_undo(new QAction)
{
    setTabsClosable(false);
//    addActions(QList<QAction*>{m_action_delete, m_action_undo, m_action_redo});
//    m_action_delete->setIcon(QIcon(":/images/delete-icon.png"));
 //   m_action_undo->setIcon(QIcon(":/images/Undo-icon.png"));
 //   m_action_redo->setIcon(QIcon(":/images/Redo-icon.png"));

//    connect(this, SIGNAL(currentCanvasChanged(Canvas*)),
//            this, SLOT(emitCurrentDiagramSceneChanged(Canvas*)));
//    connect(this, &dunnart::CanvasTabWidget::currentCanvasChanged, [=](Canvas *canvas){
//        qDebug() << "emit currentDiagramSceneChanged";
//        emit currentDiagramSceneChanged(qobject_cast<DiagramScene*>(canvas));
//    });
}

int DiagramSceneTabWidget::newDiagramScene(QString title, QString boxType, QString arrowType)
{
    DiagramScene *scene = new DiagramScene(boxType, arrowType);
    connect(scene, SIGNAL(initArrangement()), this, SIGNAL(initArrangement()));
    DiagramView *view = new DiagramView(scene);
    //m_undo_group->addStack(scene->undoStack());
    connect(view, SIGNAL(zoomChanged()), this, SIGNAL(zoomChanged()));

    return addTab(view, title);
}

DiagramScene *DiagramSceneTabWidget::currentDiagramScene()
{
    return qobject_cast<DiagramScene*>(diagramSceneAt(currentIndex()));
}

DiagramView *DiagramSceneTabWidget::currentDiagramView()
{
    return qobject_cast<DiagramView*>(currentWidget());
}

DiagramScene *DiagramSceneTabWidget::diagramSceneAt(int index)
{
    return qobject_cast<DiagramScene*>(diagramViewAt(index)->scene());
}

DiagramView *DiagramSceneTabWidget::diagramViewAt(int index)
{
    return qobject_cast<DiagramView*>(widget(index));
}

void DiagramSceneTabWidget::emitCurrentDiagramSceneChanged(QGraphicsScene *canvas)
{
    DiagramScene* scene = qobject_cast<DiagramScene*>(canvas);
    if (scene)
        emit currentDiagramSceneChanged(scene);
}
