#include "lazynutlistcombobox.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"
#include <QDebug>
#include <QMouseEvent>
#include <QShowEvent>
#include <QCoreApplication>
#include <QPoint>


LazyNutListComboBox::LazyNutListComboBox(QString getListCmd, QWidget *parent) :
    getListCmd(getListCmd), eventSwitch(true), QComboBox(parent)
{
    setEditable(false);
    eventParams.clear();
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
    if (!SessionManager::instance()->isReady())
    {
        emit listReady();
        repostEvent();
    }
    else
    {
        savedCurrentText = currentText();
        getListCmd = cmd.isEmpty() ? getListCmd : cmd;
        if (!getListCmd.isEmpty())
        {
            LazyNutJobParam *param = new LazyNutJobParam;
            param->logMode |= ECHO_INTERPRETER; // debug purpose
            param->cmdList = QStringList({QString("xml %1").arg(getListCmd)});
            param->answerFormatterType = AnswerFormatterType::ListOfValues;
            param->setAnswerReceiver(this, SLOT(buildList(QStringList)));
            SessionManager::instance()->setupJob(param, sender());
        }
        else
        {
            emit listReady();
            repostEvent();
        }
    }
}


bool LazyNutListComboBox::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        // if left click
        if (mouseEvent->button() == Qt::LeftButton &&
            mouseEvent->buttons() == Qt::LeftButton &&
            mouseEvent->modifiers() == Qt::NoModifier
            )
        {
            // if not waiting for data from lazyNut
            if (eventMutex.tryLock())
            {
                // if event generated outside, i.e. not reposted from this class,
                // i.e. if lazyNut has not been called yet on this event
                if (eventSwitch)
                {
                    eventParams.type = mouseEvent->type();
                    eventParams.localPos = mouseEvent->pos();
                    eventParams.button = mouseEvent->button();
                    eventParams.buttons = mouseEvent->buttons();
                    eventParams.modifiers = mouseEvent->modifiers();

                    getList();
                    // eventMutex unlocked in repostEvent()
                    // eventSwitch assigned to false in repostEvent()
                }
                else // the event has been reposted from this class
                {
                    // pass event to base class event handler
                    eventSwitch = true;
                    eventMutex.unlock();
                    return QComboBox::event(event);
                }
            }
            // else do nothing, return true at the end
        }
        else // pass event to base class event handler
            return QComboBox::event(event);

        break;
    }
    case QEvent::MouseButtonDblClick:
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        // ignore left double clicks: a double click has the same effect of a single click
        if (mouseEvent->button() == Qt::LeftButton &&
            mouseEvent->buttons() == Qt::LeftButton &&
            mouseEvent->modifiers() == Qt::NoModifier
            )
            break; // return true at the end

        else
            return QComboBox::event(event);
    }

    case QEvent::Show:
    {
        if (eventMutex.tryLock())
        {
            if (eventSwitch)
            {
                eventParams.type = event->type();
                getList();
            }
            else
            {
                eventSwitch = true;
                eventMutex.unlock();
                return QComboBox::event(event);
            }
        }
        else // pass event to base class event handler
            return QComboBox::event(event);

        break;
    }
    default:
        return QComboBox::event(event);
    }
    return true;
}

void LazyNutListComboBox::buildList(QStringList list)
{
    clear();
    addItems(list);
    setCurrentText(savedCurrentText);
    emit listReady();
    repostEvent();
}


void LazyNutListComboBox::repostEvent()
{
    if (eventSwitch) // safety, should always be true
    {
        eventSwitch = false;
        eventMutex.unlock();
        switch (eventParams.type)
        {
        case QEvent::MouseButtonPress:
        {
            QCoreApplication::postEvent(this, new QMouseEvent(eventParams.type, eventParams.localPos,
                                                              eventParams.button, eventParams.buttons, eventParams.modifiers));
            break;
        }
        case QEvent::Show:
        {
            QCoreApplication::postEvent(this, new QShowEvent());
            break;
        }
        default:
            ;
        }
        eventParams.clear();
    }
}



void LazyNutListComboBox::__debug_getList(QString /*cmd*/)
{
    // a debug version of getList()
    // at every call it creates a new layer starting from A and proceeding alphabetically
    // and asks lazyNut the list of layers. In this way the list is always different at every call
    // and the size increases.
    if (!SessionManager::instance()->isReady())
    {
        emit listReady();
        repostEvent();
    }
    else
    {
        savedCurrentText = currentText();
        QStringList list;
        list.append(QString("create layer %1").arg(QChar('A' + __debug_getList_counter++)));
        list.append("xml list layer");
        LazyNutJobParam *param = new LazyNutJobParam;
        param->logMode |= ECHO_INTERPRETER; // debug purpose
        param->cmdList = list;
        param->answerFormatterType = AnswerFormatterType::ListOfValues;
        param->setAnswerReceiver(this, SLOT(buildList(QStringList)));
        SessionManager::instance()->setupJob(param, sender());
    }
}
