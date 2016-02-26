#include "easyNetMainWindow.h"
#include <QApplication>
#include <QFont>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    MainWindow* mainWindow = MainWindow::instance();
    mainWindow->build();
    mainWindow->showMaximized();

    int ret = app.exec();

    delete mainWindow;
    return ret;
}
