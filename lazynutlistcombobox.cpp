#include "lazynutlistcombobox.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QPoint>

LazyNutListComboBox::LazyNutListComboBox(QString getListCmd, QWidget *parent) :
    getListCmd(getListCmd), QComboBox(parent)
{
    setEditable(false);
    counter = 0;
}

void LazyNutListComboBox::setEmptyItem(bool present)
{
    if (present && !itemText(0).isEmpty())
        insertItem(0, "");

    else if (!present && itemText(0).isEmpty())
        removeItem(0);
}

void LazyNutListComboBox::setGetListCmd(QString cmd)
{
    getListCmd = cmd;
}

void LazyNutListComboBox::getList(QString cmd)
{
//    qDebug() << "get list";
//    buildList(QStringList({"badger", "mushroom", "snake"}));
    QStringList list;
    list.append(QString("create layer %1").arg(QChar('A' + counter++)));
    list.append("xml list layer");
//    getListCmd = cmd.isEmpty() ? getListCmd : cmd;
//    if (!getListCmd.isEmpty())
//    {
        LazyNutJobParam *param = new LazyNutJobParam;
        param->logMode |= ECHO_INTERPRETER; // debug purpose
        param->cmdList = list;
        //param->cmdList = QStringList({QString("xml %1").arg(getListCmd)});
        param->answerFormatterType = AnswerFormatterType::ListOfValues;
        param->setAnswerReceiver(this, SLOT(buildList(QStringList)));
        SessionManager::instance()->setupJob(param, sender());
//    }
//    else
//    {
//        clear();
//        emit listReady();
        //    }
}

void LazyNutListComboBox::savePos(const QPoint &pos)
{
     mousePos = pos;
//     qDebug () << static_cast<QMouseEvent*>(savedEvent)->button();
//     qDebug() << savedEvent->button() << savedEvent->buttons();
}

void LazyNutListComboBox::buildList(QStringList list)
{
    clear();
    addItems(list);
    emit listReady();
    QMouseEvent *event = new QMouseEvent( QEvent::MouseButtonPress,
                                          QPointF(mousePos),
                                          Qt::LeftButton,
                                          Qt::NoButton,
                                          Qt::NoModifier);
     QCoreApplication::postEvent(this, event);
//             qDebug() << savedEvent->button() << savedEvent->buttons();

//    QComboBox::mousePressEvent(savedEvent);
//    qDebug () << static_cast<QMouseEvent*>(savedEvent)->button();
//    QComboBox::event(savedEvent);
//    QComboBox::event(new QEvent(QEvent::MouseButtonPress));
//    QComboBox::event(new QEvent(QEvent::MouseButtonRelease));
}
