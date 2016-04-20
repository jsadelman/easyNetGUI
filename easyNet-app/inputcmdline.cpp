#include "inputcmdline.h"
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

InputCmdLine::InputCmdLine(QWidget *parent)
    : QLineEdit(parent)
{
    connect(this,SIGNAL(returnPressed()),
            this,SLOT(sendCommand()));
    QFont qf("Courier");
    setFont(qf);
    setStyleSheet( "border: 0px" ) ;

    installEventFilter(this);
}

void InputCmdLine::sendCommand()
{
    QString line = text();
    emit commandReady(line);
    clear();
}

bool InputCmdLine::eventFilter(QObject* obj, QEvent *event)
{
    static bool tabFlag;

//    if (obj == this)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

            if(keyEvent->key() != Qt::Key_Tab)
                tabFlag = false;

            if (keyEvent->key() == Qt::Key_Up)
            {
//                 qDebug() << "lineEdit -> Qt::Key_Up";
                 emit historyKey(-1, text());
                 return true;
            }
            else if(keyEvent->key() == Qt::Key_Down)
            {
                emit historyKey(1, text());
                return true;
            }
            else if(keyEvent->key() == Qt::Key_Escape)
            {
//                qDebug() << "lineEdit -> Qt::Key_Escape";
                emit historyKey(0, text());
                return true;
            }
            else if(keyEvent->key() == Qt::Key_Tab)
            {
                if (tabFlag)
                    emit historyKey(200, text());
                else
                    emit historyKey(100, text());
                tabFlag = true;
                return true;
            }
        }
        return false;
    }
    return eventFilter(obj, event);
}
