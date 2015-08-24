#include "diagramscenetabwidget.h"
#include "diagramscene.h"
#include "libdunnartcanvas/canvasview.h"
#include "diagramview.h"

#include <QUndoGroup>
#include <QDebug>

using dunnart::CanvasView;
using dunnart::Canvas;

DiagramSceneTabWidget::DiagramSceneTabWidget(QMainWindow *window)
    : CanvasTabWidget(window)
{
    setTabsClosable(false);
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
    m_undo_group->addStack(scene->undoStack());

    qDebug() << "Adding tab for diagram scene" << title;
    return addTab(view, title);
//    return insertTab(INT_MAX, view, title);
//    setCurrentWidget(canvasview);
}

DiagramScene *DiagramSceneTabWidget::currentDiagramScene()
{
    return qobject_cast<DiagramScene*>(currentCanvas());
}

DiagramView *DiagramSceneTabWidget::currentDiagramView()
{
    return qobject_cast<DiagramView*>(currentCanvasView());
}

DiagramScene *DiagramSceneTabWidget::diagramSceneAt(int index)
{
    return qobject_cast<DiagramScene*>(canvasViewAt(index)->canvas());
}

DiagramView *DiagramSceneTabWidget::diagramViewAt(int index)
{
    return qobject_cast<DiagramView*>(canvasViewAt(index));
}

void DiagramSceneTabWidget::emitCurrentDiagramSceneChanged(Canvas *canvas)
{
    DiagramScene* scene = qobject_cast<DiagramScene*>(canvas);
    if (scene)
        emit currentDiagramSceneChanged(scene);
}
