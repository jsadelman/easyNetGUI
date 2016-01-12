#include "dataviewer.h"
#include "ui_dataviewer.h"

#include <QAction>



DataViewer::DataViewer(QWidget *parent)
    : QDockWidget(parent), ui(nullptr)
{

}

DataViewer::~DataViewer()
{
    delete ui;
}

void DataViewer::setUi(Ui_DataViewer *ui_dataViewer)
{
    ui = ui_dataViewer;
    if (!ui)
        return;
    ui->setupUi(this);
    connect(ui, SIGNAL(deleteItemRequested(QString)), this, SLOT(removeItem(QString)));
    connect(ui, SIGNAL(currentItemChanged(QString)), this, SLOT(updateCurrentItem(QString)));
}

void DataViewer::enableActions(bool enable)
{
    if (!ui)
        return;
    ui->saveAct->setEnabled(enable);
    ui->copyAct->setEnabled(enable);
    //..
}

