#include "dataviewer.h"



DataViewer::DataViewer(QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(parent, flags), ui(nullptr)
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



}

