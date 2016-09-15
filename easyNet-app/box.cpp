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
    : QObject(),
      DiagramItem(DiagramItem::Layer,"",new QMenu,0),
      m_longNameToDisplayIntact("Orthographic"),
      m_widthMarginProportionToLongestLabel(0.1),
      m_labelPointSize(9),
      default_input_observer_Rex("input_channel ([^)]*)\\) default_observer\\)"),
      enabledObserverSet()
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    labelFont = QFont("Anka Coder", 10); //, QFont::Bold  scene() ? scene()->font() : QFont();
    connect(this, SIGNAL(lazyNutTypeChanged()), this, SLOT(setupDefaultObserverFilter()));
    m_ports.clear();
    setZValue(10);
    autoFontSize();
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
//    if (boxWidth != 0)
//        position *= (autoWidth() / boxWidth);
//    autoSize();
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

void Box::autoFontSize()
{
    QFontMetrics *fm = new QFontMetrics(labelFont);
    bool enlarge = mywidth >= (1.0 + 2.0 * m_widthMarginProportionToLongestLabel) * fm->width(m_longNameToDisplayIntact);
    while (enlarge == mywidth >= (1.0 + 2.0 * m_widthMarginProportionToLongestLabel) * fm->width(m_longNameToDisplayIntact))
    {
        labelFont.setPointSize(labelFont.pointSize() + (enlarge ? 1 : -1));
        delete fm;
        fm = new QFontMetrics(labelFont);
    }
    labelFont.setPointSize(labelFont.pointSize() + (enlarge ? -1 : 1));
    delete fm;
}

void Box::setFillColour(QColor q)
{
  QBrush br;
  br.setColor(q);
  br.setStyle(Qt::SolidPattern);
  setBrush(br);
}

void Box::setDashedStroke(bool arg)
{
  if(arg)
  {
      m_pen.setStyle(Qt::DashLine);
   }
  else{
      m_pen.setStyle(Qt::SolidLine);
  }
}

void Box::disableAllObservers()
{
    foreach(QString observer, observerOfDataView.values().toSet())
    {
        enableObserver(observer, false);
    }
    observerOfDataView.clear();
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
    painter->setPen(QPen(Qt::black,4));
    painter->setFont(labelFont);
    painter->setBackground(Qt::transparent);
//    if (canvas())
//    {
//        QFont labelFont = canvas()->canvasFont();
//        labelFont.setPointSize(14);
//        painter->setFont(labelFont);
//    }
    painter->setRenderHint(QPainter::TextAntialiasing, true);
//    QString displayLabel = limitString(m_label, m_longNameToDisplayIntact.length());
    painter->drawText(labelBoundingRect(), Qt::AlignCenter | Qt::TextWordWrap, m_label);
}


void Box::setLabel(QString label)
{
    if (label.isEmpty())
    {
        label = name();
        label.replace("_", " ");
    }
    m_label = label;
//    m_label = limitString(label, m_longNameToDisplayIntact.length());
    update();
}


QRectF Box::labelBoundingRect() const
{
    return boundingRect().adjusted(width()*m_widthMarginProportionToLongestLabel, 0,
                                   -width()*m_widthMarginProportionToLongestLabel, 0);
}

void Box::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (m_lazyNutType != "layer")
        return;

    QMap<QString, QString> portPrettyName;
    QMap<QString, QString> dataViewNameStub;
    foreach (QString observer, defaultObservers.keys())
    {
        if (observer.contains("input_channel"))
        {
            default_input_observer_Rex.indexIn(observer);
            portPrettyName[observer] = default_input_observer_Rex.cap(1);
            dataViewNameStub[observer] = QString("%1.%2").arg(m_name).arg(portPrettyName[observer]);
        }
        else if (observer.contains("default_observer"))
        {
            portPrettyName[observer] = "state";
            dataViewNameStub[observer] = QString("%1.state").arg(m_name);
        }
    }

    QMenu menu;
    QList<QAction*> dataViewActionList;
    foreach (QString dataView, QStringList({"Tables", "Plots"}))
    {
        QMenu *dataViewMenu = menu.addMenu(dataView);
        foreach (QString observer, defaultObservers.keys())
        {
            QMenu *portMenu = dataViewMenu->addMenu(portPrettyName[observer]);
            foreach(QString dataViewTypePath, dataViewTypesPath(dataView, portPrettyName[observer]))
            {
                QString dataViewType = QFileInfo(dataViewTypePath).baseName();
                QMap <QString, QVariant> dataViewData;
                dataViewData["dataView"] = dataView_lazyNut(dataView);
                dataViewData["observer"] = observer;
                dataViewData["dataViewType"] = dataViewTypePath;
                dataViewData["dataViewName"] = QString("%1.%2").arg(dataViewNameStub.value(observer)).arg(dataViewType);
                QString prettyName = dataViewNameStub.value(observer);
                prettyName.replace('.', ' ');
                dataViewData["prettyName"] = prettyName;
                dataViewActionList.append(portMenu->addAction(dataViewType));
                dataViewActionList.last()->setData(dataViewData);
                dataViewActionList.last()->setCheckable(true);
                dataViewActionList.last()->setChecked(observerOfDataView.contains(dataViewData["dataViewName"].toString()));
            }
        }
    }
    QString transfer = layerTransfer();
    QAction *lesionAct = menu.addAction(tr("Lesion layer"));
    lesionAct->setVisible(transfer != "lesion_transfer");
    QAction *unlesionAct = menu.addAction(tr("Unlesion layer"));
    unlesionAct->setVisible(transfer == "lesion_transfer");

    menu.addAction("Properties", this, SIGNAL(propertiesRequested()));

    QAction *action = Q_NULLPTR;
    if (!menu.isEmpty())
    {
        QApplication::restoreOverrideCursor();
        action = menu.exec(event->screenPos());
    }

    if (dataViewActionList.contains(action))
    {
        QMap <QString, QVariant> dataViewData = action->data().toMap();
        if (action->isChecked())
        {
            if (!observerOfDataView.contains(dataViewData.value("dataViewName").toString()))
            {
                observerOfDataView.insert(dataViewData.value("dataViewName").toString(), dataViewData.value("observer").toString());
                dataViewFilter->addName(dataViewData.value("dataViewName").toString());
            }
            enableObserver(dataViewData.value("observer").toString(), true);
            if (!SessionManager::instance()->descriptionCache->exists(dataViewData.value("dataViewName").toString()))
            {
                QMap<QString,QString> settings;
                settings["df"] = dataViewData.value("observer").toString();
                SessionManager::instance()->createDataView(dataViewData.value("dataViewName").toString(),
                                                           dataViewData.value("prettyName").toString(),
                                                           dataViewData.value("dataView").toString(),
                                                           dataViewData.value("dataViewType").toString(),
                                                           settings);
            }
        }
        else
        {
            observerOfDataView.remove(dataViewData.value("dataViewName").toString());
            if (!observerOfDataView.values().contains(dataViewData.value("observer").toString()))
                enableObserver(dataViewData.value("observer").toString(), false);
        }
    }

    else if (action == lesionAct)
        lesionBox(true);

    else if (action == unlesionAct)
        lesionBox(false);
}

void Box::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton)
        return;
    if (!observerOfDataView.isEmpty())
        emit focusOnPlotRequested(observerOfDataView.keys().first());
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
        for (int row = 0; row < defaultObserverFilter->rowCount(); ++row)
        {
            QString observer = defaultObserverFilter->data(defaultObserverFilter->index(row, ObjectCache::NameCol)).toString();
            defaultObserverUpdater->requestObject(observer);
        }

        dataViewFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
        dataViewFilter->setFilterKeyColumn(ObjectCache::NameCol);
        connect(dataViewFilter, &ObjectCacheFilter::objectDestroyed, [=](QString name)
        {
            QString observer = observerOfDataView.value(name);
            if (!observer.isEmpty())
            {
                observerOfDataView.remove(name);
                if (!observerOfDataView.values().contains(observer))
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

QStringList Box::dataViewTypesPath(QString dataView, QString port)
{
    QDomDocument *description = SessionManager::instance()->description(m_name);
    QStringList list;
    if (description)
    {
        XMLelement elem = XMLelement(*description)["hints"][dataView_hint(dataView)];
        if (elem.isString())
        {
            XMLelement portElem = elem["port"];
            if (portElem.isNull() || portElem() == port)
                list << elem();
        }
        else if (elem.isList())
        {
            XMLelement listElem = elem.firstChild("string");
            while (!listElem.isNull())
            {
                XMLelement portElem = listElem["port"];
                if (portElem.isNull() || portElem() == port)
                    list << listElem();
                listElem = listElem.nextSibling("string");
            }
        }
    }
    return list;
}

QString Box::layerTransfer()
{
    QDomDocument *description = SessionManager::instance()->description(m_name);
    return description ? XMLelement(*description)["subtype"]["layer_transfer"]() : QString();
}
