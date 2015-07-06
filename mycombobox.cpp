#include <QtGui>
#include "mycombobox.h"

myComboBox::myComboBox(QWidget *parent)
{

}

myComboBox::~myComboBox()
{

}

void myComboBox::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "Entered dragEnterEvent in myComboBox";
    clearEditText();

//    if (event->mimeData()->hasFormat("application/easyNet"))
//    {
//        qDebug() << event->mimeData()->text();
        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            addItem(event->mimeData()->text());
            setCurrentIndex(findData(event->mimeData()->text(), Qt::DisplayRole));
            event->acceptProposedAction();
        }

//    }
//    else
//        event->ignore();
}

void myComboBox::dropEvent(QDropEvent *event)
{
    qDebug() << "Entered dropEvent in myComboBox";
    clearEditText();

    if (event->mimeData()->hasFormat("application/easyNet"))
    {
        QByteArray itemData = event->mimeData()->data("application/easyNet");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        QPoint offset;
        dataStream  >> offset;

    }
    else
        event->ignore();
}
