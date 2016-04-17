#include "arrow.h"
#include "sessionmanager.h"
#include "xmlelement.h"
#include "objectcache.h"
#include "box.h"
#include <QDebug>
#include <QMenu>
#include <QApplication>
#include <QGraphicsSceneEvent>

Arrow::Arrow()
{
}

void Arrow::setNewEndpoint(Arrow::End, QPointF, Box *, Arrow::Strategy)
{

}

void Arrow::initWithConnection(Box *, Box *)
{

}

void Arrow::setDashedStroke(bool)
{

}

void Arrow::updatePosition()
{

}

QAction *Arrow::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu &menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QString subtype;
    QDomDocument *domDoc = SessionManager::instance()->descriptionCache->getDomDoc(m_name);
    if (domDoc)
        subtype =  XMLelement(*domDoc)["subtype"]();


    QAction *lesionAct = menu.addAction(tr("Lesion connection"));
//    lesionAct->setVisible(m_lazyNutType == "connection" && subtype != "lesioned_connection");
    lesionAct->setVisible(m_lazyNutType == "connection" && !dashedStroke());
    QAction *unlesionAct = menu.addAction(tr("Unlesion connection"));
//    unlesionAct->setVisible(m_lazyNutType == "connection" && subtype == "lesioned_connection");
     unlesionAct->setVisible(m_lazyNutType == "connection" && dashedStroke());

     QAction *action = NULL;
     if (!menu.isEmpty())
     {
         QApplication::restoreOverrideCursor();
         action = menu.exec(event->screenPos());
     }


    if (action == lesionAct)
        lesion();

    else if (action == unlesionAct)
        unlesion();

    return action;
}

void Arrow::lesion()
{
    QString cmd = QString("%1 lesion").arg(m_name);
    SessionManager::instance()->runCmd(cmd);
    // basic version (should check if cmd was executed succesfully)
    setDashedStroke(true);

}


void Arrow::unlesion()
{
    QString cmd = QString("%1 unlesion").arg(m_name);
    SessionManager::instance()->runCmd(cmd);
    // basic version (should check if cmd was executed succesfully)
    setDashedStroke(false);
}

