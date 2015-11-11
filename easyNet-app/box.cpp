#include "box.h"
#include "sessionmanager.h"
#include "lazynutjobparam.h"
#include "libdunnartcanvas/limitstring.h"
#include "objectcachefilter.h"


#include <QPainter>
#include <QDebug>


const QColor layerCol = QColor(240, 240, 210);
const QColor representationCol = QColor("azure");
const QColor observedCol = QColor("white");

using dunnart::limitString;

Box::Box()
    : ShapeObj("rect"),
      m_longNameToDisplayIntact("longname"),
      m_widthMarginProportionToLongestLabel(0.1),
      m_widthOverHeight(1.618),
      m_labelPointSize(9),
      default_input_observer_Rex("default_input_observer (\\d+)")
{
    labelFont = canvas() ? canvas()->canvasFont() : QFont();
    connect(this, SIGNAL(lazyNutTypeChanged()), this, SLOT(setupDefaultDataframesFilter()));
    m_ports.clear();


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

void Box::setLabelPointSize(int labelPointSize)
{
     m_labelPointSize = labelPointSize;
     labelFont.setPointSize(m_labelPointSize);
}


void Box::read(const QJsonObject &json, qreal boxWidth)
{
    m_name = json["name"].toString();
    m_lazyNutType = json["lazyNutType"].toString();
    QPointF position(json["x"].toDouble(),json["y"].toDouble());
    if (boxWidth != 0)
        position *= (autoWidth() / boxWidth);
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

qreal Box::autoWidth()
{
    QFontMetrics fm(labelFont);
    return (1.0 + 2.0 * m_widthMarginProportionToLongestLabel) * fm.width(m_longNameToDisplayIntact);
}

void Box::autoSize()
{
//   QFontMetrics fm(labelFont);
//   qreal autoWidth = (1.0 + 2.0 * m_widthMarginProportionToLongestLabel) * fm.width(m_longNameToDisplayIntact);
   qreal autoHeigth = autoWidth() / m_widthOverHeight;
   cmd_setSize(QSizeF(autoWidth(), autoHeigth));
}

void Box::paintLabel(QPainter *painter)
{
    painter->setPen(Qt::black);
    painter->setFont(labelFont);
//    if (canvas())
//    {
//        QFont labelFont = canvas()->canvasFont();
//        labelFont.setPointSize(14);
//        painter->setFont(labelFont);
//    }
    painter->setRenderHint(QPainter::TextAntialiasing, true);
//    QString displayLabel = limitString(m_label, m_longNameToDisplayIntact.length());
    painter->drawText(labelBoundingRect(), Qt::AlignCenter | Qt::TextSingleLine, m_label);
}


void Box::setLabel(const QString& label)
{
    m_label = limitString(label, m_longNameToDisplayIntact.length());
    update();
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
        else if (m_name.startsWith("feature"))
            return "plot_features";
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

            QString dataframe = defaultDataframesFilter->data(defaultDataframesFilter->index(row, ObjectCache::NameCol)).toString();
            QString observer = dataframe;
            observer.remove(QRegExp("^\\(| default_dataframe\\)"));
            QMap <QString, QVariant> plotData; // QMap <QString, QString> is not allowed
            plotData["dataframe"] = dataframe;
            plotData["observer"] = observer;
            QString rplotName;
            QString prettyName;
            if (observer.contains("default_observer"))
            {
                plotData["rplotName"] = QString("%1.%2.state.plot")
                        .arg(SessionManager::instance()->currentModel())
                        .arg(m_name);
                plotData["prettyName"] = "state";
            }
            else if (observer.contains("default_input_observer"))
            {
                int i = default_input_observer_Rex.indexIn(observer);
                if (i < 0)
                    qDebug() << "cannot find default input port";
                else
                {
                    plotData["prettyName"] = m_ports.value(default_input_observer_Rex.cap(1));
                    plotData["rplotName"] = QString("%1.%2.%3.plot")
                            .arg(SessionManager::instance()->currentModel())
                            .arg(m_name)
                            .arg(plotData["prettyName"].toString());
                }
            }


//            QString plotName = defaultDataframesFilter->data(defaultDataframesFilter->index(row, ObjectCache::NameCol)).toString();
//            plotName.remove(QRegExp("\\(|_observer|\\).*"));
//            plotName.replace(" ", ".");
//            plotName.prepend(QString("%1.").arg(SessionManager::instance()->currentModel()));
//            plotName.append(".plot");
            actionList.append(plotMenu->addAction(plotData["prettyName"].toString()));
            actionList.at(row)->setCheckable(true);
            actionList.at(row)->setChecked(SessionManager::instance()->descriptionCache->exists(plotData["rplotName"].toString()));
            actionList.at(row)->setData(plotData);
        }


        menu.addMenu(plotMenu);
        QAction *lesionAct = menu.addAction(tr("Lesion layer"));
        lesionAct->setVisible(true);
        QAction *unlesionAct = menu.addAction(tr("Unlesion layer"));
        unlesionAct->setVisible(true);


//        QMenu *enableObserverMenu = new QMenu("Observer");
//        QAction *enableObserverAct = enableObserverMenu->addAction(tr("Enable default observer"));
//        QAction *disableObserverAct = enableObserverMenu->addAction(tr("Disable default observer"));
//        menu.addMenu(enableObserverMenu);

        QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);

        for (int row = 0; row < defaultDataframesFilter->rowCount(); ++row)
        {
            if (action == actionList.at(row))
            {
//                QString dataframe = defaultDataframesFilter->data(
//                            defaultDataframesFilter->index(row, ObjectCache::NameCol)).toString();
//                QString observer = dataframe;
//                observer.remove(QRegExp("^\\(| default_dataframe\\)"));
                QMap <QString, QVariant> plotData = action->data().toMap();
                if (action->isChecked())
                {
                    enableObserver(plotData.value("observer").toString());
                    defaultPlot(plotData.value("rplotName").toString(), plotData.value("dataframe").toString());
                    setFillColour(observedCol);
                    return action;
                }
                else
                {
                    disableObserver(plotData.value("observer").toString());
                    SessionManager::instance()->runCmd(QString("destroy %1").arg(plotData.value("rplotName").toString()));
                    emit plotDestroyed(plotData.value("rplotName").toString());
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

        if (action == lesionAct)
            lesion();

        else if (action == unlesionAct)
            unlesion();

        return action;

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
        defaultDataframesFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
        QRegExp rex(QString("^\\(\\(%1 .*observer.*default_dataframe\\)$").arg(m_name));
        defaultDataframesFilter->setFilterRegExp(rex);
        defaultDataframesFilter->setFilterKeyColumn(ObjectCache::NameCol);
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

void Box::lesion()
{
    QString cmd = QString("%1 lesion").arg(m_name);
    SessionManager::instance()->runCmd(cmd);
    // basic version (should check if cmd was executed succesfully)
    setDashedStroke(true);
}

void Box::unlesion()
{
    QString cmd = QString("%1 unlesion").arg(m_name);
    SessionManager::instance()->runCmd(cmd);
    // basic version (should check if cmd was executed succesfully)
    setDashedStroke(false);
}

