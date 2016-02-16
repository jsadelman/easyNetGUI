#include "box.h"
#include "sessionmanager.h"
#include "lazynutjobparam.h"
#include "libdunnartcanvas/limitstring.h"
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

using dunnart::limitString;

Box::Box()
    : ShapeObj("rect"),
      m_longNameToDisplayIntact("longname"),
      m_widthMarginProportionToLongestLabel(0.1),
      m_widthOverHeight(1.618),
      m_labelPointSize(9),
      default_input_observer_Rex("default_input_observer (\\d+)"),
      defaultObserverSet()
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
        layerFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
        layerUpdater = new ObjectUpdater(this);
        layerUpdater->setProxyModel(layerFilter);

        connect(layerUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)), this, SLOT(cacheDefaultPlotTypes(QDomDocument*)));
//        connect(layerUpdater, &ObjectUpdater::objectUpdated, [=](QDomDocument* domdoc,QString name)
//        {
//            qDebug() <<  "ObjectUpdater::objectUpdated" << (domdoc != 0) << name;
//        });
        layerFilter->setName(m_name);
        layerUpdater->requestObject(m_name);

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

void Box::cacheDefaultPlotTypes(QDomDocument *description)
{
    m_defaultPlotTypes.clear();
    if (m_lazyNutType != "layer" || !description)
        return;

    XMLelement plotTypeElem = XMLelement(*description)["hints"]["plot_type"];
    if (plotTypeElem.isString())
        m_defaultPlotTypes << XMLelement(*description)["hints"]["plot_type"]();
    else if (plotTypeElem.isList())
        m_defaultPlotTypes << XMLelement(*description)["hints"]["plot_type"].listValues();
    m_defaultPlotTypes.replaceInStrings(QRegExp("\\.R$"), "");
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

        for (int row = 0; row < defaultObserverFilter->rowCount(); ++row)
        {
            QString observer = defaultObserverFilter->data(defaultObserverFilter->index(row, ObjectCache::NameCol)).toString();
            QString dataframe = QString("(%1 default_dataframe)").arg(observer);

            QString portPrettyName, rplotName;
            if (observer.contains("default_observer"))
            {
                portPrettyName = "state";
                rplotName = QString("%1.state").arg(m_name);
            }
            else if (observer.contains("default_input_observer"))
            {
                int i = default_input_observer_Rex.indexIn(observer);
                portPrettyName = m_ports.value(default_input_observer_Rex.cap(1));
                rplotName = QString("%1.%2").arg(m_name).arg(portPrettyName);
            }
            QMenu *portMenu = plotMenu->addMenu(portPrettyName);
            foreach(QString plotType, defaultPlotTypes())
            {
                QMap <QString, QVariant> plotData; // QMap <QString, QString> is not allowed
                plotData["dataframe"] = dataframe;
                plotData["observer"] = observer;
                plotData["plotType"] = plotType;
                rplotName.append(QString(".%1").arg(plotType)).remove(QRegExp("\\.R$"));
                plotData["rplotName"] = rplotName;
                actionList.append(portMenu->addAction(plotType));
                actionList.last()->setData(plotData);
                actionList.last()->setCheckable(true);
                actionList.last()->setChecked(observerOfPlot.contains(rplotName));


            }





//            if (observer.contains("default_observer"))
//            {
//                plotData["rplotName"] = QString("%1.state").arg(m_name);
//                plotData["portPrettyName"] = "state";
//            }
//            else if (observer.contains("default_input_observer"))
//            {
//                int i = default_input_observer_Rex.indexIn(observer);
//                if (i < 0)
//                    qDebug() << "cannot find default input port";
//                else
//                {
//                    plotData["portPrettyName"] = m_ports.value(default_input_observer_Rex.cap(1));
//                    plotData["rplotName"] = QString("%1.%2")
//                            .arg(m_name)
//                            .arg(plotData["portPrettyName"].toString());
//                }
//            }







//            if (!observerOfPlot.contains(plotData["rplotName"].toString()))
//            {
//                observerOfPlot.insert(plotData["rplotName"].toString(), observer);
//                plotFilter->addName(plotData["rplotName"].toString());
//            }

//            actionList.append(plotMenu->addAction(plotData["portPrettyName"].toString()));
//            actionList.at(row)->setCheckable(true);
//            QDomDocument *domDoc = SessionManager::instance()->descriptionCache->getDomDoc(observer);
//            bool enabled = false;
//            if (domDoc)
//                enabled = XMLelement(*domDoc)["Enabled"]() == "1";
//            actionList.at(row)->setChecked(enabled);
//            actionList.at(row)->setData(plotData);
        }

        QString layerTransfer;
        QDomDocument *domDoc = SessionManager::instance()->descriptionCache->getDomDoc(m_name);
        if (domDoc)
            layerTransfer =  XMLelement(*domDoc)["subtype"]["layer_transfer"]();

        menu.addMenu(plotMenu);
        QAction *lesionAct = menu.addAction(tr("Lesion layer"));
        lesionAct->setVisible(layerTransfer != "lesion_transfer");
        QAction *unlesionAct = menu.addAction(tr("Unlesion layer"));
        unlesionAct->setVisible(layerTransfer == "lesion_transfer");

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
                    emit createNewRPlot(plotData.value("rplotName").toString(),
                                        plotData.value("plotType").toString().append(".R"),
                                        settings, Plot_AnyTrial);
                }
                return action;
            }
            else
            {
                observerOfPlot.remove(plotData.value("rplotName").toString());
                enableObserver(plotData.value("observer").toString(), false);
                //                    SessionManager::instance()->runCmd(QString("destroy %1").arg(plotData.value("rplotName").toString()));
                //                    emit plotDestroyed(plotData.value("rplotName").toString());
                return action;
            }
        }


        if (action == lesionAct)
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
        QRegExp rex(QString("^\\(%1 .*observer[^)]*\\)$").arg(m_name));
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
            bool enabled = XMLelement(*domDoc)["Enabled"]() == "1";
            if (enabled)
                defaultObserverSet.insert(observer);
            else
                defaultObserverSet.remove(observer);

            if (defaultObserverSet.isEmpty())
                setFillColour(layerCol);
            else
                setFillColour(observedCol);
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
                    enableObserver(observerOfPlot.value(name), false);
            }
//            updateObservedState();
        });
    }
}

void Box::enableObserver(QString observer, bool enable)

{
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
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
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList << QString("%1 %2").arg(m_name).arg(lesion ? "lesion" : "unlesion");
    QList<LazyNutJob *> jobs =  QList<LazyNutJob *> ()
                                << job
                                << SessionManager::instance()->recentlyModifiedJob();
    SessionManager::instance()->submitJobs(jobs);
    setDashedStroke(lesion);
}


