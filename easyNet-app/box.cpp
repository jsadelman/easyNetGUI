#include "box.h"
#include "sessionmanager.h"

#include <QDebug>

Box::Box() : ShapeObj("rect")
{
}


void Box::read(const QJsonObject &json)
{
    m_name = json["name"].toString();
    m_lazyNutType = json["lazyNutType"].toString();
    QPointF position(json["x"].toDouble(),json["y"].toDouble());
    if (!position.isNull())
        setCentrePos(position);
}

void Box::write(QJsonObject &json) const
{
    json["name"] = m_name;
    json["lazyNutType"] = m_lazyNutType;
    QPointF position = centrePos();
    json["x"] = position.x();
    json["y"] = position.y();
}

QAction *Box::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu &menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QAction *enableObserverAct = menu.addAction(tr("Enable default observer"));
    enableObserverAct->setVisible(m_lazyNutType == "layer");
    QAction *disableObserverAct = menu.addAction(tr("Disable default observer"));
    disableObserverAct->setVisible(m_lazyNutType == "layer");

    QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);

    if (action == enableObserverAct)
        enableObserver();

    else if (action == disableObserverAct)
        disableObserver();

    return action;
}

void Box::enableObserver()
{
    QString cmd = QString("(%1 default_observer) enable").arg(m_name);
    SessionManager::instance()->runCmd(cmd);
}

void Box::disableObserver()
{
    QString cmd = QString("(%1 default_observer) disable").arg(m_name);
    SessionManager::instance()->runCmd(cmd);
}

