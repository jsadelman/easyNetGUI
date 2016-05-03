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
    emptyScene=new DiagramScene("nul","nul");
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
    if(held.contains(currentIndex()))
    {
        return qobject_cast<DiagramScene*>(qobject_cast<DiagramView*>(held[currentIndex()])->scene());
    }
    return qobject_cast<DiagramScene*>(diagramSceneAt(currentIndex()));
}

DiagramView *DiagramSceneTabWidget::currentDiagramView()
{
    if(held.contains(currentIndex()))
    {
        return qobject_cast<DiagramView*>(held[currentIndex()]);
    }
    return qobject_cast<DiagramView*>(currentWidget());
}

DiagramScene *DiagramSceneTabWidget::diagramSceneAt(int index)
{
    if(held.contains(index))
    {
        return qobject_cast<DiagramScene*>(qobject_cast<DiagramView*>(held[index])->scene());
    }
    return qobject_cast<DiagramScene*>(diagramViewAt(index)->scene());
}

DiagramView *DiagramSceneTabWidget::diagramViewAt(int index)
{
    if(held.contains(index))
    {
        return qobject_cast<DiagramView*>(held[index]);
    }
    return qobject_cast<DiagramView*>(widget(index));
}

void DiagramSceneTabWidget::poke()
{
    for(auto x:held) x->update();
}

void DiagramSceneTabWidget::useFake(int idx, bool yn)
{
    if(yn)
    {
      held.insert(idx,widget(idx));
      QString lab=tabText(idx);
      insertTab(idx,new DiagramView(emptyScene),lab);
      removeTab(idx+1);
      setCurrentIndex(idx);
    }
    else
    {
        qDebug()<<"returning";
        QString lab=tabText(idx);
        QWidget* todel=widget(idx);
        QWidget* returned=held[idx];
        insertTab(idx,returned,lab);
        removeTab(idx+1);
        delete todel;
        setCurrentIndex(idx);
        held.remove(idx);
    }
}

void DiagramSceneTabWidget::emitCurrentDiagramSceneChanged(QGraphicsScene *canvas)
{
    DiagramScene* scene = qobject_cast<DiagramScene*>(canvas);
    if (scene)
        emit currentDiagramSceneChanged(scene);
}
