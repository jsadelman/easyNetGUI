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
#include <QDebug>

PlotViewer::PlotViewer(QString easyNetHome, QWidget* parent)
    : easyNetHome(easyNetHome), QMainWindow(parent)
{
    plotPanel = new QTabWidget;

//    svgWidget = new QSvgWidget(this);
//    svgWidget->show();
//    plots.push_back(new QSvgWidget(this));
//    numPlots = 0;
//    currentPlotIdx = 0;
//    addPlot();
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

void PlotViewer::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(refreshAct);
    fileToolBar->addAction(snapshotAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    titleLabel = new QLabel("");

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(settingsAct);
    editToolBar->addAction(copyAct);
    navigationToolBar = addToolBar(tr("Plots"));
    navigationToolBar->addSeparator();
    navigationToolBar->addWidget(titleLabel);
}

void PlotViewer::createActions()
{
    settingsAct = new QAction(QIcon(":/images/plot_settings.png"), tr("&Settings"), this);
    settingsAct->setShortcuts(QKeySequence::Refresh);
    settingsAct->setStatusTip(tr("Plot settings"));
    connect(settingsAct, SIGNAL(triggered()), this, SIGNAL(showPlotSettings()));

    refreshAct = new QAction(QIcon(":/images/refresh.png"), tr("&Refresh"), this);
    refreshAct->setShortcuts(QKeySequence::Refresh);
    refreshAct->setStatusTip(tr("Refresh plot"));
    connect(refreshAct, &QAction::triggered, [=](){
       emit sendDrawCmd(plotName[static_cast<QSvgWidget*>(plotPanel->currentWidget())]);
    });

    snapshotAct = new QAction(QIcon(":/images/snapshot-icon.png"), tr("Snapshot"), this);
    snapshotAct->setStatusTip(tr("Snapshot"));
    connect(snapshotAct, SIGNAL(triggered()), this, SLOT(snapshot()));

//    renameAct =  new QAction(QIcon(":/images/rename-icon.png"), tr("Rename"), this);
//    renameAct->setStatusTip(tr("Rename"));
//    connect(renameAct, SIGNAL(triggered()), this, SLOT(renamePlot()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open"), this);
    openAct->setStatusTip(tr("Load plot"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(loadSVGFile()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setStatusTip(tr("Save plot"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setStatusTip(tr("Copy plot to clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copySVGToClipboard()));

}

void PlotViewer::loadSVGFile()
{
    // bring up file dialog
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load SVG File"),
                                                    easyNetHome,
                                                    tr("SVG Files (*.svg)"));
    if (!fileName.isEmpty())
    {
//        plots[plotPanel->currentIndex()]->load(fileName);
        titleLabel->setText(fileName);
    }
}

void PlotViewer::loadByteArray(QString name, QByteArray _byteArray)
{
    byteArray = _byteArray;
    QSvgWidget* svg = plotName.key(name);
    if (svg)
    {
        svg->load(byteArray);
        plotPanel->setCurrentWidget(svg);
        titleLabel->setText(name);
    }
}

void PlotViewer::save()
{
//    if (byteArray->isNull())
//        return;

    // bring up file dialog
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save SVG File"),
                                                    easyNetHome,
                                                    tr("SVG Files (*.svg)"));
    if (!fileName.isEmpty())
    {
//
          QFile file(fileName);
          file.open(QIODevice::WriteOnly);
          file.write(byteArray);
          file.close();
    }

}

void PlotViewer::copySVGToClipboard()
{
//    QPixmap* currPixmap = new QPixmap(plots[plotPanel->currentIndex()]->width(),
//                                      plots[plotPanel->currentIndex()]->height());
//    currPixmap->fill(Qt::transparent);

//    QClipboard *p_Clipboard = QApplication::clipboard();
//    p_Clipboard->setPixmap(*currPixmap);



}

void PlotViewer::addPlot(QString name)
{
    QSvgWidget *svg = new QSvgWidget;
    plotName[svg] = name;
    plotIsActive[svg] = true;
    plotPanel->addTab(svg, QString("Plot %1").arg(++progressiveTabIdx));
    plotPanel->setCurrentWidget(svg);
}

void PlotViewer::updateActivePlots()
{
//    foreach(QString plotName,plotMap.values())
//        emit sendDrawCmd(plotName);
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

void PlotViewer::resizeTimeout()
{
   resizeTimer->stop();
   emit resized(plotPanel->size());
   if(plotPanel->currentIndex()>-1)
   {
     emit sendDrawCmd(plotName[static_cast<QSvgWidget*>(plotPanel->currentWidget())]);
   }
}

void PlotViewer::freeze(QSvgWidget* svg)
{
    if (!svg)
        svg = static_cast<QSvgWidget*>(plotPanel->currentWidget());

    plotIsActive[svg] = false;
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
    if (plotIsActive[svg])
        emit setPlot(plotName[svg]);
    settingsAct->setEnabled(plotIsActive[svg]);
    refreshAct->setEnabled(plotIsActive[svg]);
    snapshotAct->setEnabled(plotIsActive[svg]);
    titleLabel->setText(plotName[svg]);
}
