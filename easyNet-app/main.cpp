#include "easyNetMainWindow.h"
#include <QApplication>
#include <QFont>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

//    QPixmap pixmap(":/splash.png");
    QPixmap pixmap(":/images/ZebLT.png");
//    QPixmap pixmap(":/images/image001.jpg");
    QSplashScreen splash(pixmap.scaledToWidth(pixmap.width()*2));
    splash.show();

    MainWindow* mainWindow = MainWindow::instance();
    mainWindow->build();
    mainWindow->showMaximized();
    //  show model chooser at startup

//    #ifdef WIN32
    if (argc == 1)
//    #endif
        mainWindow->loadModel();
//     #ifdef WIN32
    else
        mainWindow->loadModel(QDir::fromNativeSeparators(argv[1]),true);
//    #endif


    splash.finish(mainWindow);
//    mainWindow->hide(); // get out of the way while user chooses model


    int ret = app.exec();

    delete mainWindow;
    return ret;
}
