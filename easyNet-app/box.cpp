#include "box.h"
#include "sessionmanager.h"
#include "lazynutjobparam.h"
#include "libdunnartcanvas/limitstring.h"

#include <QPainter>
#include <QDebug>

using dunnart::limitString;

Box::Box()
    : ShapeObj("rect"),
      m_longNameToDisplayIntact("longnameof_level"),
      m_widthMarginProportionToLongestLabel(0.1),
      m_widthOverHeight(1.618)
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

void Box::autoSize()
{
   QFontMetrics fm(canvas()->canvasFont());
   qreal autoWidth = (1.0 + 2.0 * m_widthMarginProportionToLongestLabel) * fm.width(m_longNameToDisplayIntact);
   qreal autoHeigth = autoWidth / m_widthOverHeight;
   cmd_setSize(QSizeF(autoWidth, autoHeigth));
}

void Box::paintLabel(QPainter *painter)
{
    qDebug() << "We are in paintLabel" << painter << Qt::black;

//    painter->setPen(Qt::black);
    if (canvas())
    {
        painter->setFont(canvas()->canvasFont());
    }
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    QString displayLabel = limitString(m_label, m_longNameToDisplayIntact.length());
    painter->drawText(labelBoundingRect(), Qt::AlignCenter | Qt::TextSingleLine, displayLabel);
}

QRectF Box::labelBoundingRect() const
{
    return boundingRect().adjusted(width()*m_widthMarginProportionToLongestLabel, 0,
                                   -width()*m_widthMarginProportionToLongestLabel, 0);
}

QString Box::defaultPlotType()
{
    if (m_lazyNutType == "layer")
        return "activity";
    else
        return "";
}


QAction *Box::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu &menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QAction *defaultPlotAct = menu.addAction(tr("Set up default plot"));
    defaultPlotAct->setVisible(m_lazyNutType == "layer");
    QAction *enableObserverAct = menu.addAction(tr("Enable default observer"));
    enableObserverAct->setVisible(m_lazyNutType == "layer");
    QAction *disableObserverAct = menu.addAction(tr("Disable default observer"));
    disableObserverAct->setVisible(m_lazyNutType == "layer");

    QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);

    if (action == defaultPlotAct)
        defaultPlot();

    else if (action == enableObserverAct)
        enableObserver();

    else if (action == disableObserverAct)
        disableObserver();

    return action;
}



void Box::defaultPlot()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({QString("(%1 default_observer) enable").arg(m_name)});
    param->setEndOfJobReceiver(this,SLOT(sendCreateNewPlotOfType()));
    SessionManager::instance()->setupJob(param, sender());
}

void Box::sendCreateNewPlotOfType()
{
    QMap<QString,QString> settings;
    settings["df"]=QString("((%1 default_observer) default_dataframe)").arg(m_name);
    QString plotName = QString("%1.%2.default").arg(
                SessionManager::instance()->currentModel(), m_name);
    emit createNewPlotOfType(plotName, QString("%1.R").arg(defaultPlotType()), settings);
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

