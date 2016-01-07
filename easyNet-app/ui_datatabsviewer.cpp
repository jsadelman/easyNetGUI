#include "ui_datatabsviewer.h"

#include <QVBoxLayout>

Ui_DataTabsViewer::Ui_DataTabsViewer(QWidget *parent)
    : Ui_DataViewer(parent)
{

}

Ui_DataTabsViewer::~Ui_DataTabsViewer()
{

}

void Ui_DataTabsViewer::setupUi(DataViewer *dataViewer)
{
    // build widget
    tabWidget = new QTabWidget;
    tabWidget->setTabsClosable(true);
    connect(tabWidget, &QTabWidget::tabCloseRequested, [=](int index)
    {
       emit deleteItemRequested(itemMap.key(tabWidget->widget(index)));
    });
    connect(tabWidget, &QTabWidget::currentChanged, [=](int index)
    {
        emit currentItemChanged(itemMap.key(tabWidget->widget(index)));
    });
    setCentralWidget(tabWidget);

    // actions
    createActions();
    createToolBars();

    connect(setDispatchModeOverrideMapper, SIGNAL(mapped(int)),
            dataViewer, SLOT(setDispatchModeOverride(int)));
    connect(setDispatchModeAutoAct, SIGNAL(triggered(bool)),
            dataViewer, SLOT(setDispatchModeAuto(bool)));
    dataViewer->setDispatchModeAuto(true);
    setDispatchModeAutoAct->setChecked(true);
    connect(openAct, SIGNAL(triggered()), dataViewer, SLOT(open()));
    connect(saveAct, SIGNAL(triggered()), dataViewer, SLOT(save()));
    connect(copyAct, SIGNAL(triggered()), dataViewer, SLOT(copy()));

}

