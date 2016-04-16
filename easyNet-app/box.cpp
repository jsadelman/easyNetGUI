#include "box.h"
#include "sessionmanager.h"
#include "lazynutjobparam.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "enumclasses.h"
#include "lazynutjob.h"
#include "easyNetMainWindow.h"
#include "trialwidget.h"
#include "xmlelement.h"

#include <QPainter>
#include <QDebug>


const QColor layerCol = QColor(240, 240, 210);
const QColor representationCol = QColor("azure");
const QColor observedCol = QColor("white");


Box::Box()
    :
      m_longNameToDisplayIntact("longname"),
      m_widthMarginProportionToLongestLabel(0.1),
      m_widthOverHeight(1.618),
      m_labelPointSize(9),
      default_input_observer_Rex("input_channel ([^)]*)\\) default_observer\\)"),
      enabledObserverSet(),
      m_defaultPlotTypes(),
      m_layerTransfer()
{
    labelFont = canvas() ? canvas()->canvasFont() : QFont();
    connect(this, SIGNAL(lazyNutTypeChanged()), this, SLOT(setupDefaultObserverFilter()));
    m_ports.clear();


}

void Box::setLazyNutType(const QString &lazyNutType)
{
    m_lazyNutType = lazyNutType;
    if (m_lazyNutType == "layer")
    {
        setFillColour(layerCol);
//        layerFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
//        layerUpdater = new ObjectUpdater(this);
//        layerUpdater->setProxyModel(layerFilter);
//        if (m_name == "letters")
//            qDebug() << Q_FUNC_INFO << "this " << this << "layerUpdater" << layerUpdater;

//        connect(layerUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)), this, SLOT(cacheDefaultPlotTypes(QDomDocument*)));
////        connect(layerUpdater, &ObjectUpdater::objectUpdated, [=](QDomDocument* domdoc,QString name)
////        {
////            qDebug() <<  "ObjectUpdater::objectUpdated" << (domdoc != 0) << name;
////        });
//        layerFilter->setName(m_name);
//        layerUpdater->requestObject(m_name);

    }

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

QStringList Box::defaultPlotTypes()
{
    if (m_lazyNutType == "layer")
        return m_defaultPlotTypes;

    return QStringList();
}

void Box::cacheFromDescription(QDomDocument *description, QString name)
{


    if (m_lazyNutType != "layer" || name != m_name || !description)
        return;
    m_defaultPlotTypes.clear();
    XMLelement plotTypeElem = XMLelement(*description)["hints"]["plot_type"];
    if (plotTypeElem.isString())
        m_defaultPlotTypes << XMLelement(*description)["hints"]["plot_type"]();
    else if (plotTypeElem.isList())
        m_defaultPlotTypes << XMLelement(*description)["hints"]["plot_type"].listValues();
    m_defaultPlotTypes.replaceInStrings(QRegExp("\\.R$"), "");

    m_layerTransfer = XMLelement(*description)["subtype"]["layer_transfer"]();
}


QAction *Box::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu &menu)
{
    // a workaround for the following bug (feature?) of Dunnart or Qt:
    // if right click on Box A without clicking on an action, then right click on Box B, the event on B is delivered to A.
    if (!contains(event->pos()))
        return nullptr;
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    if (m_lazyNutType == "layer")
    {
        QMenu *plotMenu = new QMenu("Plot");
        QList<QAction*> actionList;
        foreach (QString observer, defaultObservers.keys())
        {
            QString dataframe = observer;
            QString portPrettyName, rplotName;
            if (observer.contains("input_channel"))
            {
//                int i = default_input_observer_Rex.indexIn(observer);
//                portPrettyName = m_ports.value(default_input_observer_Rex.cap(1));
                int i = default_input_observer_Rex.indexIn(observer);
                portPrettyName = default_input_observer_Rex.cap(1);
                rplotName = QString("%1.%2").arg(m_name).arg(portPrettyName);
            }
            else if (observer.contains("default_observer"))
            {
                portPrettyName = "state";
                rplotName = QString("%1.state").arg(m_name);
            }
            QMenu *portMenu = plotMenu->addMenu(portPrettyName);
            foreach(QString plotType, defaultPlotTypes())
            {
                QMap <QString, QVariant> plotData; // QMap <QString, QString> is not allowed
                plotData["dataframe"] = dataframe;
                plotData["observer"] = observer;
                plotData["plotType"] = plotType;
                plotData["rplotName"] = QString("%1.%2").arg(rplotName).arg(plotType).remove(QRegExp("\\.R$"));
                actionList.append(portMenu->addAction(plotType));
                actionList.last()->setData(plotData);
                actionList.last()->setCheckable(true);
                actionList.last()->setChecked(observerOfPlot.contains(plotData["rplotName"].toString()));
            }
        }
        menu.addMenu(plotMenu);
        QAction *lesionAct = menu.addAction(tr("Lesion layer"));
        lesionAct->setVisible(m_layerTransfer != "lesion_transfer");
        QAction *unlesionAct = menu.addAction(tr("Unlesion layer"));
        unlesionAct->setVisible(m_layerTransfer == "lesion_transfer");

        QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);

        if (actionList.contains(action))
        {
            QMap <QString, QVariant> plotData = action->data().toMap();
            if (action->isChecked())
            {
                if (!observerOfPlot.contains(plotData.value("rplotName").toString()))
                {
                    observerOfPlot.insert(plotData.value("rplotName").toString(), plotData.value("observer").toString());
                    plotFilter->addName(plotData.value("rplotName").toString());
                }
                enableObserver(plotData.value("observer").toString(), true);
                if (!SessionManager::instance()->descriptionCache->exists(plotData.value("rplotName").toString()))
                {
                    QMap<QString,QString> settings;
                    settings["df"] = plotData.value("dataframe").toString();
                    emit createDataViewRequested(   plotData.value("rplotName").toString(),
                                                    "rplot",
                                                    plotData.value("plotType").toString().append(".R"),
                                                    settings,
                                                    false);
                }
            }
            else
            {
                observerOfPlot.remove(plotData.value("rplotName").toString());
                if (!observerOfPlot.values().contains(plotData.value("observer").toString()))
                    enableObserver(plotData.value("observer").toString(), false);
            }
        }

        else if (action == lesionAct)
            lesionBox(true);

        else if (action == unlesionAct)
            lesionBox(false);

        return action;
    }
    else
    {
        QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);
        return action;
    }
}




void Box::setupDefaultObserverFilter()
{
    if (m_lazyNutType == "layer")
    {
        defaultObserverFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
        QRegExp rex(QString("^\\(+%1 .*default_observer\\)$").arg(m_name));
        defaultObserverFilter->setFilterRegExp(rex);
        defaultObserverFilter->setFilterKeyColumn(ObjectCache::NameCol);
        defaultObserverUpdater = new ObjectUpdater(this);
        defaultObserverUpdater->setProxyModel(defaultObserverFilter);
        for (int row = 0; row < defaultObserverFilter->rowCount(); ++row)
        {
            QString observer = defaultObserverFilter->data(defaultObserverFilter->index(row, ObjectCache::NameCol)).toString();
            defaultObserverUpdater->requestObject(observer);
        }
        connect(defaultObserverUpdater, &ObjectUpdater::objectUpdated, [=](QDomDocument* domDoc, QString observer)
        {
            defaultObservers[observer] = XMLelement(*domDoc)["Enabled"]() == "1";
            bool anyEnabled = false;
            foreach(bool enabled, defaultObservers.values())
                anyEnabled |= enabled;
            if (anyEnabled)
                setFillColour(observedCol);
            else
                setFillColour(layerCol);
        });

        plotFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
        plotFilter->setFilterKeyColumn(ObjectCache::NameCol);
        connect(plotFilter, &ObjectCacheFilter::objectDestroyed, [=](QString name)
        {
            QString observer = observerOfPlot.value(name);
            if (!observer.isEmpty())
            {
                observerOfPlot.remove(name);
                if (!observerOfPlot.values().contains(observer))
                    enableObserver(observer, false);
            }
//            updateObservedState();
        });
    }
}

void Box::enableObserver(QString observer, bool enable)

{
    LazyNutJob *job = new LazyNutJob;
    job->cmdList << QString("%1 %2").arg(observer).arg(enable? "enable" : "disable");
    QMap<QString, QVariant> jobData;
    jobData.insert("observer", observer);
    jobData.insert("enabled", enable);
    job->data = jobData;
    job->appendEndOfJobReceiver(SessionManager::instance(), SLOT(observerEnabled()));
    QList<LazyNutJob *> jobs =  QList<LazyNutJob *> ()
                                << job
                                << SessionManager::instance()->recentlyModifiedJob();
    SessionManager::instance()->submitJobs(jobs);
}


void Box::lesionBox(bool lesion)
{
    LazyNutJob *job = new LazyNutJob;
    job->cmdList << QString("%1 %2").arg(m_name).arg(lesion ? "lesion" : "unlesion");
    QList<LazyNutJob *> jobs =  QList<LazyNutJob *> ()
                                << job
                                << SessionManager::instance()->recentlyModifiedJob();
    SessionManager::instance()->submitJobs(jobs);
    setDashedStroke(lesion);
}


