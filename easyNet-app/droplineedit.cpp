#include "droplineedit.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

DropLineEdit::DropLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setAcceptDrops(true);
}

void DropLineEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}

void DropLineEdit::dropEvent(QDropEvent *event)
{
    clear();
    setText(event->mimeData()->text());
    event->acceptProposedAction();
}
