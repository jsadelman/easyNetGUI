#include "box.h"
#include "sessionmanager.h"
#include "lazynutjobparam.h"
#include "libdunnartcanvas/limitstring.h"
#include "objectcataloguefilter.h"


#include <QPainter>
#include <QDebug>


const QColor layerCol = QColor(240, 240, 210);
const QColor representationCol = QColor("azure");
const QColor observedCol = QColor("white");

using dunnart::limitString;

Box::Box()
    : ShapeObj("rect"),
      m_longNameToDisplayIntact("longnameof_level"),
      m_widthMarginProportionToLongestLabel(0.1),
      m_widthOverHeight(1.618)
{
    connect(this, SIGNAL(lazyNutTypeChanged()), this, SLOT(setupDefaultDataframesFilter()));

}

void Box::setLazyNutType(const QString &lazyNutType)
{
    m_lazyNutType = lazyNutType;
    if (m_lazyNutType == "layer")
        setFillColour(layerCol);

    else if (m_lazyNutType == "representation")
        setFillColour(representationCol);

    emit lazyNutTypeChanged();
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
    painter->setPen(Qt::black);
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
        if (m_name == "spatial_code") //quick and dirty hack!!!
            return "spactivity-3";
        else
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
    if (m_lazyNutType == "layer")
    {
        QMenu *plotMenu = new QMenu("Plot");
        QList<QAction*> actionList;
        for (int row = 0; row < defaultDataframesFilter->rowCount(); ++row)
        {
            // from:
            // ((my_level default_input_observer 0) default_dataframe)
            // to:
            // my_model.my_level.default_input.0.plot
            QString plotName = defaultDataframesFilter->data(defaultDataframesFilter->index(row, ObjectCatalogue::NameCol)).toString();
            plotName.remove(QRegExp("\\(|_observer|\\).*"));
            plotName.replace(" ", ".");
            plotName.prepend(QString("%1.").arg(SessionManager::instance()->currentModel()));
            plotName.append(".plot");
            actionList.append(plotMenu->addAction(plotName));
            actionList.at(row)->setCheckable(true);
            actionList.at(row)->setChecked(ObjectCatalogue::instance()->exists(plotName));
        }


        menu.addMenu(plotMenu);

//        QMenu *enableObserverMenu = new QMenu("Observer");
//        QAction *enableObserverAct = enableObserverMenu->addAction(tr("Enable default observer"));
//        QAction *disableObserverAct = enableObserverMenu->addAction(tr("Disable default observer"));
//        menu.addMenu(enableObserverMenu);

        QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);

        for (int row = 0; row < defaultDataframesFilter->rowCount(); ++row)
        {
            if (action == actionList.at(row))
            {
                QString dataframe = defaultDataframesFilter->data(
                            defaultDataframesFilter->index(row, ObjectCatalogue::NameCol)).toString();
                QString observer = dataframe;
                observer.remove(QRegExp("^\\(| default_dataframe\\)"));
                if (action->isChecked())
                {
                    enableObserver(observer);
                    defaultPlot(action->text(), dataframe);
                    setFillColour(observedCol);
                    return action;
                }
                else
                {
                    disableObserver(observer);
                    SessionManager::instance()->runCmd(QString("destroy %1").arg(action->text()));
                    emit plotDestroyed(action->text());
                    // if all actions unchecked restore original box colour
                    bool unobserved = true;
                    foreach (QAction *a, actionList)
                        unobserved &= !a->isChecked();

                    if (unobserved)
                        setFillColour(layerCol);

                    return action;
                }
            }
        }

//        else if (action == enableObserverAct)
//            enableObserver();

//        else if (action == disableObserverAct)
//            disableObserver();


    }
    else
    {
        QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);
        return action;
    }
}



void Box::defaultPlot(QString plotName, QString dataframe)
{
    QMap<QString,QString> settings;
    settings["df"] = dataframe;
    emit createNewPlotOfType(plotName, QString("%1.R").arg(defaultPlotType()), settings);
}

//void Box::sendCreateNewPlotOfType()
//{
//    QMap<QString,QString> settings;
//    settings["df"]=QString("((%1 default_observer) default_dataframe)").arg(m_name);
//    QString plotName = QString("%1.%2.default").arg(
//                SessionManager::instance()->currentModel(), m_name);
//    emit createNewPlotOfType(plotName, QString("%1.R").arg(defaultPlotType()), settings);
//}

void Box::setupDefaultDataframesFilter()
{
    if (m_lazyNutType == "layer")
    {
//        qDebug() << "in setupDefaultDataframesFilter";
        defaultDataframesFilter = new ObjectCatalogueFilter(this);
        QRegExp rex(QString("^\\(\\(%1 .*observer.*default_dataframe\\)$").arg(m_name));
        defaultDataframesFilter->setFilterRegExp(rex);
        defaultDataframesFilter->setFilterKeyColumn(ObjectCatalogue::NameCol);
    }
}

void Box::enableObserver(QString observer)
{
    QString cmd = QString("%1 enable").arg(observer);
    SessionManager::instance()->runCmd(cmd);
}

void Box::disableObserver(QString observer)
{
    QString cmd = QString("%1 disable").arg(observer);
    SessionManager::instance()->runCmd(cmd);
}

