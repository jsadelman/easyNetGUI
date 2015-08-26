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

PlotViewer::PlotViewer(QString _easyNetHome, QWidget* parent)
    : QMainWindow(parent)
{
    easyNetHome = _easyNetHome;
    plotPanel = new QTabWidget;

    svgWidget = new QSvgWidget(this);
    svgWidget->show();
//    plots.push_back(new QSvgWidget(this));
    numPlots = 0;
    currentPlotIdx = 0;
//    addPlot();
    setCentralWidget(plotPanel);

    createActions();
    createToolBars();

}

PlotViewer::~PlotViewer()
{

}

void PlotViewer::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(refreshAct);
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

    refreshAct = new QAction(QIcon(":/images/reload.png"), tr("&Refresh"), this);
    refreshAct->setShortcuts(QKeySequence::Refresh);
    refreshAct->setStatusTip(tr("Refresh plot"));
//    connect(refreshAct, SIGNAL(triggered()), this, SIGNAL(sendDrawCmd()));
    connect(refreshAct, &QAction::triggered, [=](){
       emit sendDrawCmd(plotMap[plotPanel->currentIndex()]);
    });
//    connect(refreshAct, SIGNAL(triggered()), this, SLOT(updateCurrentPlot()));

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
        plots[plotPanel->currentIndex()]->load(fileName);
        titleLabel->setText(fileName);
    }
}

void PlotViewer::loadByteArray(QString name, QByteArray _byteArray)
{
    byteArray = _byteArray;

    plots[plotMap.key(name)]->load(byteArray);
    titleLabel->setText(name);
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
    QPixmap* currPixmap = new QPixmap(plots[plotPanel->currentIndex()]->width(),
                                      plots[plotPanel->currentIndex()]->height());
    currPixmap->fill(Qt::transparent);

    plots[plotPanel->currentIndex()]->render(currPixmap, QPoint(), QRegion(), QWidget::DrawChildren);
    QClipboard *p_Clipboard = QApplication::clipboard();
    p_Clipboard->setPixmap(*currPixmap);



}

void PlotViewer::addPlot(QString name)
{
    plots.push_back(new QSvgWidget(this));
    numPlots++;
    currentPlotIdx = numPlots-1;
    int idx = plotPanel->addTab(plots[numPlots-1], tr("Plot ")+QString::number(numPlots));
    qDebug() << "adding plot to panel. New numPlots = " << numPlots;
    plotPanel->setCurrentIndex(idx);
    plotMap[idx] = name;
}

void PlotViewer::updateActivePlots()
{
    foreach(QString plotName,plotMap.values())
        emit sendDrawCmd(plotName);

}
