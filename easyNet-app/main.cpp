#include "easyNetMainWindow.h"
#include <QApplication>
#include <QFont>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

//    QPixmap pixmap(":/splash.png");
    QPixmap pixmap(":/images/zebra.png");
//    QPixmap pixmap(":/images/image001.jpg");
    QSplashScreen splash(pixmap);
    splash.show();
    app.processEvents();

    MainWindow* mainWindow = MainWindow::instance();
    mainWindow->build();
    mainWindow->showMaximized();
    splash.finish(mainWindow);

    int ret = app.exec();

    delete mainWindow;
    return ret;
}
