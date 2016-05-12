#include "plotviewer.h"
#include "ui_dataviewer.h"
#include "sessionmanager.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "easyNetMainWindow.h"
//#include "plotsettingswindow.h"
#include "xmlelement.h"
#include "lazynutjob.h"
#include "plotviewerdispatcher.h"


#include <QDebug>
#include <QTimer>
#include <QDomDocument>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QAction>
#include <QToolBar>
#include <QSvgWidget>


FullScreenSvgDialog::FullScreenSvgDialog(QWidget *parent)
    :QDialog(parent, Qt::FramelessWindowHint)
{
    svg = new QSvgWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    QPushButton *closeBtn = new QPushButton("Close");
    QHBoxLayout *hlayout = new QHBoxLayout;
    QSpacerItem *item = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
    hlayout->addSpacerItem(item);
    hlayout->addWidget(closeBtn);

    layout->addLayout(hlayout);
    layout->addWidget(svg);
    setLayout(layout);
    connect (closeBtn, SIGNAL(clicked()),this,SLOT(close()));
}

void FullScreenSvgDialog::loadByteArray(QByteArray byteArray)
{
    svg->load(byteArray);
}

void FullScreenSvgDialog::clearSvg()
{
    QByteArray byteArray;
    loadByteArray(byteArray);
}




PlotViewer::PlotViewer(Ui_DataViewer *ui, QWidget *parent)
    : DataViewer(ui, parent), pend(false), fullScreen(false), plotAspectRatio(1)
{
    connect(descriptionUpdater, &ObjectUpdater::objectUpdated, [=](QDomDocument*, QString name)
    {
       plotIsUpToDate[name] = false;
       updateActivePlots();
       if (dispatcher)
           dispatcher->updateInfo(name);
    });
//    connect(descriptionUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)), this, SLOT(updateDependencies(QDomDocument*,QString)));

    resizeTimer = new QTimer(this);
    connect(resizeTimer,SIGNAL(timeout()),this,SLOT(resizeTimeout()));
    // an ObjectUpdater is used otherwise an ObjectCacheFilter would send three times objectModified after an object is modified,
    // i.e. invalid true, invalid false, domDoc. In fact, all dataframes are also updated elsewhere.
    // In the near future it will be possible to setSubtype, thus saving descriptions.
    dependenciesFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    dependenciesUpdater = new ObjectUpdater(this);
    dependenciesUpdater->setProxyModel(dependenciesFilter);
    connect(dependenciesUpdater, &ObjectUpdater::objectUpdated, [=](QDomDocument*, QString name)
    {
        checkDependencies(name);
    });
    dependenciesFilter->setType("dataframe");


//    plotDescriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
//    plotDescriptionFilter->setFilterKeyColumn(ObjectCache::NameCol);
//    plotDescriptionUpdater = new ObjectUpdater(this);
//    plotDescriptionUpdater->setProxyModel(plotDescriptionFilter);
//    connect(plotDescriptionFilter, SIGNAL(objectCreated(QString,QString,QString, QDomDocument*)),
//            this, SLOT(generatePrettyName(QString,QString,QString, QDomDocument*)));
//    descriptionFilter->setType("xfile");
    addExtraActions();
    fullScreenSvgDialog = new FullScreenSvgDialog(this);
    auto temp=QApplication::desktop()->availableGeometry();
    fullScreenSize = QSize(temp.width(),temp.height());
    fullScreenAspectRatio = double(temp.width())/temp.height();
    fullScreenSvgDialog->resize(fullScreenSize);

    connect(ui->mainWidget, SIGNAL(resizeEventOccured(QResizeEvent*)), this, SLOT(restartTimer()));
}

PlotViewer::~PlotViewer()
{
}

//QString PlotViewer::plotType(QString name)
//{
//    if (QDomDocument *description = SessionManager::instance()->descriptionCache->getDomDoc(name))
//        return QFileInfo(XMLelement(*description)["Type"]()).fileName();

//    return QString();
//}

void PlotViewer::sendPlotCmd(QString name)
{
    if (name.isEmpty() || !SessionManager::instance()->exists(name))
    {
        eNerror << "name is empty or does not exist:" << name;
        return;
    }

    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("%1 get %2").arg(name).arg(fullScreen ? fullScreenAspectRatio : plotAspectRatio)});
    job->setAnswerReceiver(this, SLOT(displaySVG(QByteArray, QString)), AnswerFormatterType::SVG);
    SessionManager::instance()->submitJobs(job);
}


void PlotViewer::updateAllActivePlots()
{
    QMapIterator<QString, bool> plotIsActiveIt(plotIsActive);
    while (plotIsActiveIt.hasNext()) {
        plotIsActiveIt.next();
        if (plotIsActiveIt.value())
        {
            plotIsUpToDate[plotIsActiveIt.key()]=false;
        }
    }
    updateActivePlots();
}

void PlotViewer::addRequestedItem(QString name, bool isBackup)
{
    if (SessionManager::instance()->descriptionCache->type(name) == "xfile")
        addItem(name, isBackup);
}

void PlotViewer::snapshot(QString name)
{
    if (name.isEmpty())
        name = ui->currentItemName();
    if (name.isEmpty())
    {
        eNwarning << "nothing to snapshot";
        return;
    }
    QString snapshotName = SessionManager::instance()->makeValidObjectName(QString("%1.Copy.1").arg(name));
    SessionManager::instance()->addToExtraNamedItems(snapshotName);
    SessionManager::instance()->setTrialRunInfo(snapshotName, SessionManager::instance()->trialRunInfo(name));
    addItem(snapshotName, true);
    if (plotIsUpToDate.value(name))
    {
        setPlotByteArray(plotByteArray.value(name), snapshotName);
    }
    else
    {
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = QStringList({QString("%1 get %2").arg(name).arg(plotAspectRatio)});
        job->setAnswerReceiver(this, SLOT(setPlotByteArray(QByteArray, QString)), AnswerFormatterType::SVG);
        SessionManager::instance()->submitJobs(job);
    }

}


void PlotViewer::destroyItem_impl(QString name)
{
    plotIsActive.remove(name);
    plotByteArray.remove(name);
    plotIsUpToDate.remove(name);
//    plotSourceModified.remove(name);
//    plotDependencies.remove(name);
}

void PlotViewer::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load SVG File"),
                                                    lastOpenDir.isEmpty() ? defaultOpenDir : lastOpenDir,
                                                    tr("SVG Files (*.svg)"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return;
        QByteArray byteArray = file.readAll();
        file.close();
        QFileInfo fi(fileName);
        QString plotName = SessionManager::instance()->makeValidObjectName(fi.completeBaseName());
        SessionManager::instance()->addToExtraNamedItems(plotName);
        addItem(plotName, false);
        displaySVG(byteArray, plotName);
        lastOpenDir = fi.path();
    }
}

void PlotViewer::save()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save current plot as SVG File"),
                                                    QString("%1/%2").arg(lastSaveDir.isEmpty() ? defaultSaveDir : lastSaveDir)
                                                    .arg(ui->currentItemName()),
                                                    tr("SVG Files (*.svg)"));
    if (!fileName.isEmpty())
    {
          QFile file(fileName);
          file.open(QIODevice::WriteOnly);
          file.write(plotByteArray.value(ui->currentItemName()));
          file.close();
          lastSaveDir = QFileInfo(fileName).path();
    }
}

void PlotViewer::copy()
{
    QSvgWidget *svg = currentSvgWidget();
    if (!svg)
        return;
    QPixmap currPixmap(svg->width(), svg->height());
    svg->render(&currPixmap);
    QApplication::clipboard()->setPixmap(currPixmap);
}

void PlotViewer::resizeTimeout()
{
    resizeTimer->stop();
    QSize size = ui->centralWidget()->size();
    plotAspectRatio = double(size.width())/size.height();
    updateAllActivePlots();
}

//void PlotViewer::dfSourceModified(QString df)
//{
//    foreach (QString plot, SessionManager::instance()->plotsOfSourceDf(df))
//    {
//        if (plotIsActive.value(plot))
//        {
////            plotSourceModified[plot] = true;
//            updateActivePlots();
//        }
//    }
//}

//void PlotViewer::generatePrettyName(QString plotName, QString type, QString subtype, QDomDocument *domDoc)
//{
//    Q_UNUSED(type)
//    Q_UNUSED(subtype)
//    Q_UNUSED(domDoc)
//    QString prettyName = plotName;
//    prettyName.remove(".plot");
//    prettyName.replace(".", " ");
//    SessionManager::instance()->setPrettyName(plotName, prettyName);
//}

void PlotViewer::setupFullScreen()
{
    fullScreen = true;
    fullScreenSvgDialog->clearSvg();
    if (plotIsActive.value(ui->currentItemName()))
    {
//        emit resized(fullScreenSize);
        sendPlotCmd();
    }
    else
    {
        fullScreenSvgDialog->loadByteArray(plotByteArray.value(ui->currentItemName()));
    }

    fullScreenSvgDialog->exec();
    fullScreen = false;
}

//void PlotViewer::addSourceDataframes(QStringList newDataframes)
//{
//    if (newDataframes.isEmpty())
//    {
//        QVariant dfVariant = SessionManager::instance()->getDataFromJob(sender(), "newDataframes");
//        if (!dfVariant.canConvert<QStringList>())
//        {
//            eNerror << "cannot retrieve a valid string from newDataframes key in sender LazyNut job";
//            return;
//        }
//        newDataframes = dfVariant.toStringList();
//        if (newDataframes.isEmpty())
//        {
//            eNerror << "QStringList from newDataframes key in sender LazyNut job is empty";
//            return;
//        }
//    }
//    foreach(QString df, newDataframes)
//    {
//        if (!SessionManager::instance()->descriptionCache->exists(df))
//        {
//            eNerror << QString(" attempt to add a non-existing dataframe %1").arg(df);
//        }
//        else
//        {
//            sourceDataframeFilter->addName(df);
//        }
//    }
//}

void PlotViewer::enableActions(bool enable)
{
    DataViewer::enableActions(enable);
//    if (!ui)
//        return;
//    bool active = plotIsActive.value(ui->currentItemName(), false);
    fullScreenAct->setEnabled(enable);
//    ui->saveAct->setEnabled(enable);
//    ui->copyAct->setEnabled(enable);
//    if (dispatcher)
//        ui->setDispatchModeAutoAct->setEnabled(enable && active);

}

void PlotViewer::setCurrentItem(QString name)
{
    DataViewer::setCurrentItem(name);
//    emit setPlotSettings(name);
    updateActivePlots();
//    if (infoVisible)
    //        showInfo(svg);
}

//void PlotViewer::updatePlot(QString name, QByteArray byteArray)
//{
//    if (fullScreen)
//        fullScreenSvgDialog->loadByteArray(byteArray);
//    else
//    {
//        QSvgWidget* svg = qobject_cast<QSvgWidget*>(ui->view(name));
//        if (svg)
//        {
//            plotIsUpToDate[name] = true;
//            plotByteArray[name] = byteArray;
//            svg->load(byteArray);
//            if (name != ui->currentItemName())
//            {
//                ui->setCurrentItem(name);
//                setCurrentItem(name);
//            }
//        }
//    }
//}

void PlotViewer::checkDependencies(QString name)
{
    foreach(QString plot, items())
    {
        if (SessionManager::instance()->dependencies(plot).contains(name))
        {
            plotIsUpToDate[plot] = false;
            updateActivePlots();
        }
    }
}

void PlotViewer::sendPlotCmd()
{
    sendPlotCmd(ui->currentItemName());
}

void PlotViewer::displaySVG(QByteArray byteArray, QString cmd)
{
    QString name = cmd.remove(QRegExp(" get .*$")).simplified();
    if (!contains(name))
    {
        eNerror << QString("attempt to display plot %1, which deos not belong to this viewer").arg(name);
        return;
    }
//    replace tags that QSvgWidget doesn't like
    byteArray.replace (QByteArray("<symbol"),QByteArray("<g     "));
    byteArray.replace (QByteArray("</symbol"),QByteArray("</g     "));
    if (fullScreen)
        fullScreenSvgDialog->loadByteArray(byteArray);
    else
    {
        QSvgWidget* svg = qobject_cast<QSvgWidget*>(ui->view(name));
        if (svg)
        {
            plotIsUpToDate[name] = true;
            plotByteArray[name] = byteArray;
            svg->load(byteArray);
            if (name != ui->currentItemName())
            {
//                ui->setCurrentItem(name);
//                setCurrentItem(name);
            }
        }
    }
}

void PlotViewer::setPlotByteArray(QByteArray byteArray, QString cmd)
{
    QString name = cmd.remove(QRegExp(" get .*$")).simplified();
    byteArray.replace (QByteArray("<symbol"),QByteArray("<g     "));
    byteArray.replace (QByteArray("</symbol"),QByteArray("</g     "));
    plotByteArray[name] = byteArray;
}

void PlotViewer::requestAddDataframe(QString name, bool isBackup)
{
    if (name.isEmpty())
    {
        QVariant v = SessionManager::instance()->getDataFromJob(sender(), "name");
        if (!v.canConvert<QString>())
        {
            eNerror << "cannot retrieve a valid string from name key in sender LazyNut job";
            return;
        }
        name = v.value<QString>();
        v = SessionManager::instance()->getDataFromJob(sender(), "isBackup");
        if (v.canConvert<bool>())
            isBackup = v.value<bool>();
    }
    if (name.isEmpty())
    {
        eNerror << "name is empty";
        return;
    }
    emit addDataframeRequested(name, isBackup);
}

void PlotViewer::restartTimer()
{
    resizeTimer->stop();
    resizeTimer->start(250);
}

//void PlotViewer::updateDependencies(QDomDocument *domDoc, QString name)
//{
//    plotDependencies.remove(name);
//    if (!domDoc)
//        return;
//    foreach (QString df, XMLelement(*domDoc)["Dependencies"].listValues())
//        plotDependencies.insert(name, df);
//}


QWidget *PlotViewer::makeView(QString name)
{
    plotIsActive[name] = SessionManager::instance()->exists(name);
    plotIsUpToDate[name] = true; // !plotIsActive[name];
//    plotSourceModified[name] = false;
//    if (plotIsActive[name])
//        updateDependencies(descriptionFilter->getDomDoc(name), name);

    return new QSvgWidget(this);
}


void PlotViewer::addNameToFilter(QString name)
{
    Q_UNUSED(name)
//    if (!SessionManager::instance()->exists(name))
//        return;
//    plotDescriptionFilter->addName(name);
//    foreach(QString df, SessionManager::instance()->plotSourceDataframes(name))
//        sourceDataframeFilter->addName(df);
//    updateActivePlots();
}

void PlotViewer::setNameInFilter(QString name)
{
    Q_UNUSED(name)

//    plotDescriptionFilter->setName(name);
//    foreach(QString df, SessionManager::instance()->plotSourceDataframes(name))
//        sourceDataframeFilter->setName(df);
}

void PlotViewer::removeNameFromFilter(QString name)
{
    Q_UNUSED(name)

//    plotDescriptionFilter->removeName(name);
//    foreach(QString df, SessionManager::instance()->plotSourceDataframes(name))
//        sourceDataframeFilter->removeName(df);
}

void PlotViewer::updateActivePlots()
{
    QString name = ui->currentItemName();
    if (plotIsActive.value(name, false) && !plotIsUpToDate.value(name, false))
    {
        if (visibleRegion().isEmpty())
        {
            pend=true;
        }
        else
        {
//            plotIsUpToDate[name] = true;
            sendPlotCmd(name);
        }
    }
    else if (!plotIsActive.value(name, false) && plotByteArray.contains(name))
    {
        displaySVG(plotByteArray.value(name), name);
//        currentSvgWidget()->load(plotByteArray.value(name));
    }

}

QSvgWidget *PlotViewer::currentSvgWidget()
{
    return qobject_cast<QSvgWidget *>(ui->currentView());
}

void PlotViewer::addExtraActions()
{
    fullScreenAct = new QAction(QIcon(":/images/Full_screen_view.png"), "Full Screen", this);
    connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(setupFullScreen()));
    fullScreenAct->setEnabled(false);
    ui->editToolBar[this]->addAction(fullScreenAct);
}


void PlotViewer::paintEvent(QPaintEvent *event)
{
    if(pend)
    {
        pend=false;
        resizeTimer->stop();
        resizeTimer->start(250);
    }
}

void PlotViewer::resizeEvent(QResizeEvent *)
{
    resizeTimer->stop();
    resizeTimer->start(250);
}

