#include "plotviewer.h"
#include "ui_dataviewer.h"
#include "sessionmanager.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "easyNetMainWindow.h"
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

    resizeTimer = new QTimer(this);
    connect(resizeTimer,SIGNAL(timeout()),this,SLOT(resizeTimeout()));

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


void PlotViewer::sendPlotCmd(QString name)
{
    if (name.isEmpty() || !SessionManager::instance()->exists(name))
    {
        eNerror << "name is empty or does not exist:" << name;
        return;
    }
    plotLastRatio[name] = fullScreen ? fullScreenAspectRatio : plotAspectRatio;
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("%1 get %2").arg(name).arg(plotLastRatio[name])});
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
//    if (SessionManager::instance()->descriptionCache->type(name) == "xfile")
//        addItem(name, isBackup);
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
    addItem(snapshotName);
    if (plotIsUpToDate.value(name))
    {
        setPlotByteArray(plotByteArray.value(name), snapshotName);
    }
    else
    {
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = QStringList({QString("%1 get %2").arg(name).arg(plotAspectRatio)});
        job->setAnswerReceiver(this, SLOT(storeCopyByteArray(QByteArray,QString)), AnswerFormatterType::SVG);
        job->appendEndOfJobReceiver(this, SLOT(assignStoredCopyByteArray()));
        QMap<QString, QVariant> jobData;
        jobData.insert("name", snapshotName);
        job->data = jobData;
        SessionManager::instance()->submitJobs(job);
    }

}


void PlotViewer::destroyItem_impl(QString name)
{
    plotIsActive.remove(name);
    plotByteArray.remove(name);
    plotIsUpToDate.remove(name);
    plotLastRatio.remove(name);
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
        addItem(plotName);
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
    double ratio = double(size.width())/size.height();
    if (ratio/plotAspectRatio < 0.9 || ratio/plotAspectRatio > 1.1)
    {
        plotAspectRatio = ratio;
        updateAllActivePlots();
    }
}



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

void PlotViewer::enableActions(bool enable)
{
    DataViewer::enableActions(enable);
    fullScreenAct->setEnabled(enable);
}

bool PlotViewer::setCurrentItem(QString name)
{
    if (!DataViewer::setCurrentItem(name))
        return false;
    if (plotIsUpToDate.value(name) && plotLastRatio.value(name, plotAspectRatio) == plotAspectRatio)
    {
        displaySVG(plotByteArray.value(name, QByteArray()), name);
    }
    else
    {
        plotIsUpToDate[name] = false;
        updateActivePlots();
    }
//    if (infoVisible)
    //        showInfo(svg);
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

void PlotViewer::storeCopyByteArray(QByteArray byteArray, QString cmd)
{
    Q_UNUSED(cmd)
    byteArray.replace (QByteArray("<symbol"),QByteArray("<g     "));
    byteArray.replace (QByteArray("</symbol"),QByteArray("</g     "));
    copyByteArray = byteArray;
}

void PlotViewer::assignStoredCopyByteArray()
{
    QVariant v = SessionManager::instance()->getDataFromJob(sender(), "name");
    if (!v.canConvert<QString>())
    {
        eNerror << "cannot retrieve a valid string from name key in sender LazyNut job";
        return;
    }
    QString name = v.value<QString>();
    plotByteArray[name] = copyByteArray;
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


QWidget *PlotViewer::makeView(QString name)
{
    plotIsActive[name] = SessionManager::instance()->exists(name);
    plotIsUpToDate[name] = true;
    return new QSvgWidget(this);
}


void PlotViewer::addNameToFilter(QString name)
{
    Q_UNUSED(name)
}

void PlotViewer::setNameInFilter(QString name)
{
    Q_UNUSED(name)
}

void PlotViewer::removeNameFromFilter(QString name)
{
    Q_UNUSED(name)
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
            plotIsUpToDate[name] = true;
            sendPlotCmd(name);
        }
    }
    else if (!plotIsActive.value(name, false) && plotByteArray.contains(name))
    {
        displaySVG(plotByteArray.value(name), name);
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

