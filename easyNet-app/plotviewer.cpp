#include "plotviewer.h"
#include <QSvgWidget>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QFile>
#include <QClipboard>
#include <QApplication>
#include <QPixmap>
#include <QInputDialog>
#include <QDebug>

PlotViewer::PlotViewer(QString easyNetHome, QWidget* parent)
    : easyNetHome(easyNetHome), progressiveTabIdx(0), QMainWindow(parent)
{
    plotPanel = new QTabWidget;

    setCentralWidget(plotPanel);

    createActions();
    createToolBars();

    connect(plotPanel, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

    resizeTimer = new QTimer(this);
    connect(resizeTimer,SIGNAL(timeout()),this,SLOT(resizeTimeout()));
}

PlotViewer::~PlotViewer()
{

}

QSvgWidget *PlotViewer::currentSvgWidget()
{
    return qobject_cast<QSvgWidget*>(plotPanel->currentWidget());
}

void PlotViewer::createToolBars()
{
    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(refreshAct);
    editToolBar->addAction(snapshotAct);
    editToolBar->addAction(settingsAct);

    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(renameAct);
    fileToolBar->addAction(copyAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(deleteAct);

    titleLabel = new QLabel("");
    navigationToolBar = addToolBar(tr("Plots"));
    navigationToolBar->addSeparator();
    navigationToolBar->addWidget(titleLabel);
}

void PlotViewer::createActions()
{
    settingsAct = new QAction(QIcon(":/images/plot_settings.png"), tr("&Settings"), this);
    settingsAct->setShortcut(QKeySequence::Refresh);
    settingsAct->setStatusTip(tr("Plot settings"));
    connect(settingsAct, SIGNAL(triggered()), this, SIGNAL(showPlotSettings()));

    refreshAct = new QAction(QIcon(":/images/refresh.png"), tr("&Refresh"), this);
    refreshAct->setShortcut(QKeySequence::Refresh);
    refreshAct->setStatusTip(tr("Refresh plot"));
    connect(refreshAct, &QAction::triggered, [=](){
       emit sendDrawCmd(plotName.value(currentSvgWidget()));
    });

    snapshotAct = new QAction(QIcon(":/images/snapshot-icon.png"), tr("Snapshot"), this);
    snapshotAct->setStatusTip(tr("Snapshot"));
    connect(snapshotAct, SIGNAL(triggered()), this, SLOT(snapshot()));

    renameAct =  new QAction(QIcon(":/images/rename-icon.png"), tr("Rename"), this);
    renameAct->setStatusTip(tr("Rename"));
    connect(renameAct, SIGNAL(triggered()), this, SLOT(renamePlot()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open"), this);
    openAct->setStatusTip(tr("Load plot"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(loadSVGFile()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setStatusTip(tr("Save plot"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    copyAct = new QAction(QIcon(":/images/clipboard.png"), tr("&Copy"), this);
    copyAct->setStatusTip(tr("Copy plot to clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copySVGToClipboard()));

    deleteAct = new QAction(QIcon(":/images/delete-icon.png"), tr("&Delete"), this);
    deleteAct->setShortcut(QKeySequence::Delete);
    deleteAct->setStatusTip(tr("Delete plot"));
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(deletePlot()));
}

void PlotViewer::loadSVGFile()
{
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load SVG File"),
                                                    easyNetHome,
                                                    tr("SVG Files (*.svg)"));
    if (!fileName.isEmpty())
    {
        QString name = QFileInfo(fileName).baseName();
        if (plotName.values().contains(name))
            name.append("_copy");

        addPlot(name);
        setPlotActive(false);
        currentSvgWidget()->load(fileName);
        currentTabChanged(plotPanel->currentIndex()); // refresh action enable properties
    }
}

void PlotViewer::loadByteArray(QString name, QByteArray _byteArray)
{
    QSvgWidget* svg = plotName.key(name);
    if (svg)
    {
        byteArray[svg] = _byteArray;
        svg->load(byteArray.value(svg));
        plotPanel->setCurrentWidget(svg);
        titleLabel->setText(name);
    }
}

void PlotViewer::save()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save SVG File"),
                                                    QString("%1/%2").arg(easyNetHome)
                                                    .arg(plotName.value(currentSvgWidget())),
                                                    tr("SVG Files (*.svg)"));
    if (!fileName.isEmpty())
    {
          QFile file(fileName);
          file.open(QIODevice::WriteOnly);
          file.write(byteArray.value(currentSvgWidget()));
          file.close();
    }
}

void PlotViewer::copySVGToClipboard()
{
    QPixmap* currPixmap = new QPixmap(currentSvgWidget()->width(),
                                      currentSvgWidget()->height());
    currentSvgWidget()->render(currPixmap);
    QClipboard *p_Clipboard = QApplication::clipboard();
    p_Clipboard->setPixmap(*currPixmap);
}

void PlotViewer::addPlot(QString name)
{
    QSvgWidget *svg = new QSvgWidget;
    plotName[svg] = name;
    plotPanel->addTab(svg, QString("Plot %1").arg(++progressiveTabIdx));
    setPlotActive(true, svg);
    plotPanel->setCurrentWidget(svg);
    currentTabChanged(plotPanel->currentIndex());
}

void PlotViewer::updateActivePlots()
{
    QMapIterator<QSvgWidget*, bool> plotIsActiveIt(plotIsActive);
    while (plotIsActiveIt.hasNext()) {
        plotIsActiveIt.next();
        if (plotIsActiveIt.value())
            emit sendDrawCmd(plotName[plotIsActiveIt.key()]);
    }
}


void PlotViewer::resizeEvent(QResizeEvent*)
{
    resizeTimer->stop();
    resizeTimer->start(250);
}


void PlotViewer::setPlotActive(bool isActive, QSvgWidget *svg)
{
    svg = svg ? svg : currentSvgWidget();
    plotIsActive[svg] = isActive;
    plotPanel->setTabIcon(plotPanel->indexOf(svg),
                          isActive ? QIcon(":/images/icon-record.png") :
                                     QIcon(":/images/snapshot-icon.png"));
}


void PlotViewer::resizeTimeout()
{
   resizeTimer->stop();
   emit resized(plotPanel->size());
   if(plotPanel->currentIndex()>-1)
   {
     emit sendDrawCmd(plotName.value(currentSvgWidget()));
   }
}


void PlotViewer::freeze(QSvgWidget* svg)
{
    if (!svg)
        svg = currentSvgWidget();

    setPlotActive(false, svg);
}

void PlotViewer::renamePlot()
{
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename plot", "New name:", QLineEdit::Normal,
                                            plotName.value(currentSvgWidget()), &ok);
    if (ok && !newName.isEmpty())
    {
        plotName[currentSvgWidget()] = newName;
        titleLabel->setText(newName);
    }
}

void PlotViewer::deletePlot()
{
    if (!plotIsActive.value(currentSvgWidget()))
    {
        plotName.remove(currentSvgWidget());
        plotIsActive.remove(currentSvgWidget());
        byteArray.remove(currentSvgWidget());
        delete currentSvgWidget();
    }
}

void PlotViewer::snapshot()
{
    freeze();
    QSvgWidget* currentSVG = static_cast<QSvgWidget*>(plotPanel->currentWidget());
    QString currentName = plotName.value(currentSVG);
    plotName[currentSVG] = QString("%1_SNAPSHOT").arg(currentName);
    addPlot(currentName);

}

void PlotViewer::currentTabChanged(int index)
{
    QSvgWidget* svg = static_cast<QSvgWidget*>(plotPanel->widget(index));
    if (plotIsActive.value(svg))
        emit setPlot(plotName.value(svg));

    settingsAct->setEnabled(plotIsActive.value(svg));
    refreshAct->setEnabled(plotIsActive.value(svg));
    snapshotAct->setEnabled(plotIsActive.value(svg));
    renameAct->setEnabled(!plotIsActive.value(svg));
    deleteAct->setEnabled(!plotIsActive.value(svg));
    titleLabel->setText(plotName.value(svg));
}
