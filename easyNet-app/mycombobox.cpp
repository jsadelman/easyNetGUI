#include <QtGui>
#include "mycombobox.h"

myComboBox::myComboBox(QWidget *parent)
{
    setAcceptDrops(true);
}

myComboBox::~myComboBox()
{

}

void myComboBox::setArg(QString arg)
{
    thisArg = arg;
}

void myComboBox::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::MoveAction);
    QRect widgetRect = this->geometry();
    if (this->selectionRect().contains(this->mapFromGlobal(QCursor::pos())))
    {
        if (!(this->currentText().isEmpty()))
            savedComboBoxText=this->currentText(); // save the text before clearing box in case user changes mind
        clearEditText();
        emit argWasChanged(thisArg);
    }
    else
        qDebug() << "Elvis has left the building";
//    event->accept();
}

void myComboBox::dropEvent(QDropEvent *event)
{
    clearEditText();
    addItem(event->mimeData()->text());
    setCurrentIndex(findData(event->mimeData()->text(),Qt::DisplayRole));

    event->setDropAction(Qt::MoveAction);
    event->accept();
}

void myComboBox::restoreComboBoxText()
{
    int idx = this->findData(savedComboBoxText,Qt::DisplayRole);
    if (idx==-1)
    {
        addItem(savedComboBoxText);
        idx = this->findData(savedComboBoxText,Qt::DisplayRole);
    }
    setCurrentIndex(idx);
//    savedComboBoxText="";

}
