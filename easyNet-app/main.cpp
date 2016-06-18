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
    splash.finish(mainWindow);
    mainWindow->showMaximized();
//    mainWindow->hide(); // get out of the way while user chooses model


    int ret = app.exec();

    delete mainWindow;
    return ret;
}
