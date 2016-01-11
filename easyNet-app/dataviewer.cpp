#include "dataviewer.h"
#include "ui_dataviewer.h"



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
}

void DataViewer::removeItem(QString name)
{
    ui->removeItem(name);
    // ... Dataframe or Plot specific bookkeeping
}

