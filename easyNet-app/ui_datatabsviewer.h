#ifndef UI_DATATABSVIEWER_H
#define UI_DATATABSVIEWER_H

#include "ui_dataviewer.h"


class QTabWidget;



class Ui_DataTabsViewer : public Ui_DataViewer
{
public:
    Ui_DataTabsViewer(QWidget * parent = 0);
    ~Ui_DataTabsViewer();
    virtual void setupUi(DataViewer *dataViewer);

private:
    QTabWidget *tabWidget;

};

#endif // UI_DATATABSVIEWER_H
