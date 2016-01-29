#include "plotviewer.h"
#include "ui_dataviewer.h"
#include "sessionmanager.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "easyNetMainWindow.h"
#include "plotsettingswindow.h"
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
    : DataViewer(ui, parent), pend(false), fullScreen(false)
{
    resizeTimer = new QTimer(this);
    connect(resizeTimer,SIGNAL(timeout()),this,SLOT(resizeTimeout()));
    sourceDataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    connect(sourceDataframeFilter, SIGNAL(objectModified(QString)), this, SLOT(dfSourceModified(QString)));
    plotDescriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    plotDescriptionFilter->setFilterKeyColumn(ObjectCache::NameCol);
    plotDescriptionUpdater = new ObjectUpdater(this);
    plotDescriptionUpdater->setProxyModel(plotDescriptionFilter);
    connect(plotDescriptionFilter, SIGNAL(objectCreated(QString,QString,QString, QDomDocument*)),
            this, SLOT(generatePrettyName(QString,QString,QString, QDomDocument*)));
    destroyedObjectsFilter->setType("xfile");
    addExtraActions();
    fullScreenSvgDialog = new FullScreenSvgDialog(this);
    auto temp=QApplication::desktop()->availableGeometry();
    fullScreenSize = QSize(temp.width(),temp.height());
    fullScreenSvgDialog->resize(fullScreenSize);
}

PlotViewer::~PlotViewer()
{
}

QString PlotViewer::plotType(QString name)
{
    if (QDomDocument *description = SessionManager::instance()->descriptionCache->getDomDoc(name))
        return QFileInfo(XMLelement(*description)["Type"]()).fileName();

    return QString();
}

bool PlotViewer::contains(QString name)
{
    return viewMap.contains(name);
}

void PlotViewer::updateAllActivePlots()
{
    QMapIterator<QSvgWidget*, bool> plotIsActiveIt(svgIsActive);
    while (plotIsActiveIt.hasNext()) {
        plotIsActiveIt.next();
        if (plotIsActiveIt.value())
        {
            svgIsUpToDate[plotIsActiveIt.key()]=false;
        }
    }
    updateActivePlots();
}

void PlotViewer::initiateRemoveItem(QString name)
{
    if (dispatcher)
    {
        dispatcher->moveFromViewerToHistory(name);
    }
    else
    {
        emit removePlotSettings(name);
        SessionManager::instance()->destroyObject(name);
    }
}

void PlotViewer::removeItem(QString name)
{
    if (!contains(name))
    {
        eNwarning << QString("attempt to delete non-existing plot %1").arg(name);
    }
    else
    {
        ui->removeItem(name);

        QSvgWidget *svg = qobject_cast<QSvgWidget*>(viewMap.value(name));
        svgIsActive.remove(svg);
        svgByteArray.remove(svg);
        svgIsUpToDate.remove(svg);
        svgSourceModified.remove(svg);
        svgTrialRunInfo.remove(svg);
        delete svg;
        viewMap.remove(name);
//        if (SessionManager::instance()->plotFlags(name) & Plot_Backup)
//        {
//            foreach(QString df, SessionManager::instance()->plotSourceDataframes(name))
//                SessionManager::instance()->destroyObject(df);
//        }

    }
}

void PlotViewer::open()
{

}

void PlotViewer::save()
{

}

void PlotViewer::copy()
{

}

//void PlotViewer::addItem(QString name, bool setCurrent)
//{
//    if (name.isEmpty())
//    {
//        QVariant v = SessionManager::instance()->getDataFromJob(sender(), "plotName");
//        if (!v.canConvert<QString>())
//        {
//            eNerror << "cannot retrieve a valid string from plotName key in sender LazyNut job";
//            return;
//        }
//        name = v.value<QString>();
//        v = SessionManager::instance()->getDataFromJob(sender(), "setCurrent");
//        if (v.canConvert<bool>())
//            setCurrent = v.value<bool>();
//    }
//    if (name.isEmpty())
//    {
//        eNerror << "name is empty";
//    }
//    else if (!SessionManager::instance()->exists(name))
//    {
//        eNerror << QString("attempt to add a non-existing plot %1").arg(name);
//    }
//    else if (viewMap.contains(name))
//    {
//        if (setCurrent)
//            ui->setCurrentItem(name);
//    }
//    else
//    {
//        QSvgWidget* svg = new QSvgWidget;
//        viewMap[name] = svg;
//        ui->addItem(name, svg);
//        svgIsActive[svg] = true;
//        svgIsUpToDate[svg] = false;
//        svgSourceModified[svg] = false;


//        plotDescriptionFilter->addName(name);
//        foreach(QString df, SessionManager::instance()->plotSourceDataframes(name))
//            sourceDataframeFilter->addName(df);
//        updateActivePlots();
////        ui->addItem(name, new QWidget(ui));
//        if (setCurrent)
//            ui->setCurrentItem(name);
//    }
//}

void PlotViewer::resizeTimeout()
{
    resizeTimer->stop();
    emit resized(ui->size());
    updateAllActivePlots();
}

void PlotViewer::dfSourceModified(QString df)
{
    foreach (QString plot, SessionManager::instance()->plotsOfSourceDf(df))
    {
        QSvgWidget* svg = qobject_cast<QSvgWidget*>(viewMap.value(plot));
        if (svgIsActive.value(svg))
        {
            svgSourceModified[svg] = true;
            updateActivePlots();
        }
    }
}

void PlotViewer::generatePrettyName(QString plotName, QString type, QString subtype, QDomDocument *domDoc)
{
    Q_UNUSED(type)
    Q_UNUSED(subtype)
    Q_UNUSED(domDoc)
    QString prettyName = plotName;
    prettyName.remove(".plot");
    prettyName.replace(".", " ");
    SessionManager::instance()->setPrettyName(plotName, prettyName);
}

void PlotViewer::setupFullScreen()
{
    fullScreen = true;
    emit resized(fullScreenSize);
    emit sendDrawCmd(ui->currentItem());
    fullScreenSvgDialog->clearSvg();
    fullScreenSvgDialog->exec();
    fullScreen = false;
}

void PlotViewer::addSourceDataframes(QStringList newDataframes)
{
    if (newDataframes.isEmpty())
    {
        QVariant dfVariant = SessionManager::instance()->getDataFromJob(sender(), "newDataframes");
        if (!dfVariant.canConvert<QStringList>())
        {
            eNerror << "cannot retrieve a valid string from newDataframes key in sender LazyNut job";
            return;
        }
        newDataframes = dfVariant.toStringList();
        if (newDataframes.isEmpty())
        {
            eNerror << "QStringList from newDataframes key in sender LazyNut job is empty";
            return;
        }
    }
    foreach(QString df, newDataframes)
    {
        if (!SessionManager::instance()->descriptionCache->exists(df))
        {
            eNerror << QString(" attempt to add a non-existing dataframe %1").arg(df);
        }
        else
        {
            sourceDataframeFilter->addName(df);
        }
    }
}

void PlotViewer::enableActions(bool enable)
{
//    DataViewer::enableActions(enable);
    if (!ui)
        return;
    bool active = svgIsActive.value(currentSvgWidget(), false);
    settingsAct->setEnabled(enable && active);
    fullScreenAct->setEnabled(enable);
    ui->saveAct->setEnabled(enable);
    ui->copyAct->setEnabled(enable);
    if (dispatcher)
        ui->setDispatchModeAutoAct->setEnabled(enable && active);

}

void PlotViewer::updateCurrentItem(QString name)
{
    DataViewer::updateCurrentItem(name);
    emit setPlotSettings(name);
    updateActivePlots();
//    if (infoVisible)
    //        showInfo(svg);
}

void PlotViewer::updatePlot(QString name, QByteArray byteArray)
{
    if (fullScreen)
        fullScreenSvgDialog->loadByteArray(byteArray);
    else
    {
        QSvgWidget* svg = qobject_cast<QSvgWidget*>(viewMap.value(name, nullptr));
        if (svg)
        {
            svgIsUpToDate[svg] = true;
            svgByteArray[svg] = byteArray;
            svg->load(byteArray);
            ui->setCurrentItem(name);
            updateCurrentItem(name);
        }
    }
}

void PlotViewer::addItem_impl(QString name)
{
    QSvgWidget* svg = new QSvgWidget;
    viewMap[name] = svg;
    svgIsActive[svg] = true;
    svgIsUpToDate[svg] = false;
    svgSourceModified[svg] = false;
}

void PlotViewer::addNameToFilter(QString name)
{
    plotDescriptionFilter->addName(name);
    foreach(QString df, SessionManager::instance()->plotSourceDataframes(name))
        sourceDataframeFilter->addName(df);
    updateActivePlots();
}

void PlotViewer::setNameInFilter(QString name)
{
    plotDescriptionFilter->setName(name);
    foreach(QString df, SessionManager::instance()->plotSourceDataframes(name))
        sourceDataframeFilter->setName(df);
}

void PlotViewer::removeNameFromFilter(QString name)
{
    plotDescriptionFilter->removeName(name);
    foreach(QString df, SessionManager::instance()->plotSourceDataframes(name))
        sourceDataframeFilter->removeName(df);
}

void PlotViewer::updateActivePlots()
{
    if (svgIsActive.value(currentSvgWidget(), false) && !svgIsUpToDate.value(currentSvgWidget(), false))
    {
        if(visibleRegion().isEmpty())
        {
            pend=true;
        }
        else
        {
            emit sendDrawCmd(ui->currentItem());
        }
    }
}

QSvgWidget *PlotViewer::currentSvgWidget()
{
    return qobject_cast<QSvgWidget *>(viewMap.value(ui->currentItem(), nullptr));
}

void PlotViewer::addExtraActions()
{
    settingsAct = new QAction(QIcon(":/images/plot_settings.png"), tr("&Settings"), this);
    settingsAct->setShortcut(QKeySequence::Refresh);
    settingsAct->setStatusTip(tr("Plot settings"));
    connect(settingsAct, SIGNAL(triggered()), this, SIGNAL(showPlotSettings()));
    settingsAct->setEnabled(false);
    ui->editToolBar->addAction(settingsAct);

    fullScreenAct = new QAction(QIcon(":/images/Full_screen_view.png"), "Full Screen", this);
    connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(setupFullScreen()));
    fullScreenAct->setEnabled(false);
    ui->editToolBar->addAction(fullScreenAct);
}

QString PlotViewer::cloneRPlot(QString name, QString newName)
{
    newName =  SessionManager::instance()->makeValidObjectName(newName.isEmpty() ? name : newName);
    // set df-related settings to values that are names of copies of the original df's
    QMap<QString, QString> sourceDataframeSettings = SessionManager::instance()->plotSourceDataframeSettings(name);
    QMutableMapIterator<QString, QString>sourceDataframeSettings_it(sourceDataframeSettings);
    while(sourceDataframeSettings_it.hasNext())
    {
        sourceDataframeSettings_it.next();
        sourceDataframeSettings_it.setValue(SessionManager::instance()->makeValidObjectName(sourceDataframeSettings_it.value()));
    }
    // make the above mentioned df copies
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER; // debug purpose
    sourceDataframeSettings_it.toFront();
    while(sourceDataframeSettings_it.hasNext())
    {
        sourceDataframeSettings_it.next();
        job->cmdList.append(QString("%1 copy %2")
                            .arg(SessionManager::instance()->plotSourceDataframeSettings(name).value(sourceDataframeSettings_it.key()))
                            .arg(sourceDataframeSettings_it.value()));
    }
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->recentlyCreatedJob();
    QMap<QString, QVariant> jobData;
    QStringList newDataframes = sourceDataframeSettings.values();
    jobData.insert("newDataframes",  newDataframes);
    jobs.last()->data = jobData;
    jobs.last()->appendEndOfJobReceiver(this, SLOT(addSourceDataframes()));
    SessionManager::instance()->submitJobs(jobs);
    // have a plot settings form created for the clone rplot
    // use the current settings from the original rplot, overwrite the df related ones and set them as defaults
    // for the new form
    QMap<QString, QString> settings = MainWindow::instance()->plotSettingsWindow->getSettings(name); // bad design but quicker than using signals/slots
    sourceDataframeSettings_it.toFront();
    while(sourceDataframeSettings_it.hasNext())
    {
        sourceDataframeSettings_it.next();
        settings[sourceDataframeSettings_it.key()] = sourceDataframeSettings_it.value();
    }
    int flags = SessionManager::instance()->plotFlags(name) | Plot_Backup;
    emit createNewRPlot(newName, plotType(name), settings, flags);
    return newName;
}

QSvgWidget *PlotViewer::newSvg(QString name)
{
    QSvgWidget* svg = new QSvgWidget;
    viewMap[name] = svg;
    ui->addItem(name, svg);
    svgIsActive[svg] = true;
    svgIsUpToDate[svg] = false;
    svgSourceModified[svg] = false;
    return svg;
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

