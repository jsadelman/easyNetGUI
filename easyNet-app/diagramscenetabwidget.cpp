#include "diagramscenetabwidget.h"
#include "diagramscene.h"
#include "libdunnartcanvas/canvasview.h"

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
    CanvasView *canvasview = new CanvasView(scene);
    m_undo_group->addStack(scene->undoStack());

    return insertTab(INT_MAX, canvasview, title);
//    setCurrentWidget(canvasview);
}

DiagramScene *DiagramSceneTabWidget::currentDiagramScene()
{
    return qobject_cast<DiagramScene*>(currentCanvas());
}

DiagramScene *DiagramSceneTabWidget::diagramSceneAt(int index)
{
    return qobject_cast<DiagramScene*>(canvasViewAt(index)->canvas());
}

void DiagramSceneTabWidget::emitCurrentDiagramSceneChanged(Canvas *canvas)
{
    qDebug() << "emit currentDiagramSceneChanged";
    emit currentDiagramSceneChanged(qobject_cast<DiagramScene*>(canvas));
}