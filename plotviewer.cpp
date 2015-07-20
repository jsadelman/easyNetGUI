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


PlotViewer::PlotViewer(QString _easyNetHome, QWidget* parent)
    : QMainWindow(parent)
{
    easyNetHome = _easyNetHome;
    svgWidget = new QSvgWidget(this);
    svgWidget->show();
    setCentralWidget(svgWidget);

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
    editToolBar->addAction(copyAct);
    navigationToolBar = addToolBar(tr("Plots"));
    navigationToolBar->addSeparator();
    navigationToolBar->addWidget(titleLabel);
}

void PlotViewer::createActions()
{
    refreshAct = new QAction(QIcon(":/images/reload.png"), tr("&Refresh"), this);
    refreshAct->setShortcuts(QKeySequence::Refresh);
    refreshAct->setStatusTip(tr("Refresh plot"));
    connect(refreshAct, SIGNAL(triggered()), this, SIGNAL(sendDrawCmd()));


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
        svgWidget->load(fileName);
        titleLabel->setText(fileName);
    }
}

void PlotViewer::load(QString name, QByteArray _byteArray)
{
    byteArray = _byteArray;
    svgWidget->load(byteArray);
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
    QPixmap* currPixmap = new QPixmap(svgWidget->width(),svgWidget->height());
    currPixmap->fill(Qt::transparent);

    svgWidget->render(currPixmap, QPoint(), QRegion(), QWidget::DrawChildren);
    QClipboard *p_Clipboard = QApplication::clipboard();
    p_Clipboard->setPixmap(*currPixmap);


}
