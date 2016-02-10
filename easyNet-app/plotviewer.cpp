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


void PlotViewer::destroyItem_impl(QString name)
{
    plotIsActive.remove(name);
    plotByteArray.remove(name);
    plotIsUpToDate.remove(name);
    plotSourceModified.remove(name);
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
        //    replace tags that QSvgWidget doesn't like
        byteArray.replace (QByteArray("<symbol"),QByteArray("<g     "));
        byteArray.replace (QByteArray("</symbol"),QByteArray("</g     "));

        QFileInfo fi(fileName);
        QString plotName = SessionManager::instance()->makeValidObjectName(fi.completeBaseName());
        SessionManager::instance()->addToExtraNamedItems(plotName);
        addItem(plotName, true);
        updatePlot(plotName, byteArray);
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
    emit resized(ui->size());
    updateAllActivePlots();
}

void PlotViewer::dfSourceModified(QString df)
{
    foreach (QString plot, SessionManager::instance()->plotsOfSourceDf(df))
    {
        if (plotIsActive.value(plot))
        {
            plotSourceModified[plot] = true;
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
    fullScreenSvgDialog->clearSvg();
    if (plotIsActive.value(ui->currentItemName()))
    {
        emit resized(fullScreenSize);
        emit sendDrawCmd(ui->currentItemName());
    }
    else
    {
        fullScreenSvgDialog->loadByteArray(plotByteArray.value(ui->currentItemName()));
    }

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
    DataViewer::enableActions(enable);
//    if (!ui)
//        return;
    bool active = plotIsActive.value(ui->currentItemName(), false);
    settingsAct->setEnabled(enable && active);
    fullScreenAct->setEnabled(enable);
//    ui->saveAct->setEnabled(enable);
//    ui->copyAct->setEnabled(enable);
//    if (dispatcher)
//        ui->setDispatchModeAutoAct->setEnabled(enable && active);

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
        QSvgWidget* svg = qobject_cast<QSvgWidget*>(ui->view(name));
        if (svg)
        {
            plotIsUpToDate[name] = true;
            plotByteArray[name] = byteArray;
            svg->load(byteArray);
            if (name != ui->currentItemName())
            {
                ui->setCurrentItem(name);
                updateCurrentItem(name);
            }
        }
    }
}


QWidget *PlotViewer::makeView(QString name)
{
    plotIsActive[name] = SessionManager::instance()->exists(name);
    plotIsUpToDate[name] = !plotIsActive[name];
    plotSourceModified[name] = false;

    return new QSvgWidget(this);
}


void PlotViewer::addNameToFilter(QString name)
{
    if (!SessionManager::instance()->exists(name))
        return;
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
    QString name = ui->currentItemName();
    if (plotIsActive.value(name, false) && !plotIsUpToDate.value(name, false))
    {
        if(visibleRegion().isEmpty())
        {
            pend=true;
        }
        else
        {
            emit sendDrawCmd(name);
        }
    }
    else if (!plotIsActive.value(name, false) && plotByteArray.contains(name))
    {
        updatePlot(name, plotByteArray.value(name));
//        currentSvgWidget()->load(plotByteArray.value(name));
    }

}

QSvgWidget *PlotViewer::currentSvgWidget()
{
    return qobject_cast<QSvgWidget *>(ui->currentView());
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
    emit createNewRPlot(newName, plotType(name), settings, flags, dispatcher ? dispatcher->info(name) : QSharedPointer<QDomDocument>());
    return newName;
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

