#include "arrow.h"
#include "sessionmanager.h"

#include <QDebug>

Arrow::Arrow() : Connector()
{
}

QAction *Arrow::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu &menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QAction *lesionAct = menu.addAction(tr("Lesion connection"));
    lesionAct->setVisible(m_lazyNutType == "connection");
    QAction *unlesionAct = menu.addAction(tr("Unlesion connection"));
    unlesionAct->setVisible(m_lazyNutType == "connection");

    QAction *action = Connector::buildAndExecContextMenu(event, menu);

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
}


void Arrow::unlesion()
{
    QString cmd = QString("%1 unlesion").arg(m_name);
    SessionManager::instance()->runCmd(cmd);
}

