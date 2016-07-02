#include "easyNetMainWindow.h"
#include <QApplication>
#include <QFont>
#include <QDebug>
#include <cstdlib>
#include <cstring>
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

    if (argc < 2)
        mainWindow->loadModel();
    else
    {
        bool complete=true;
        QString modelName;
        for(int i=1;i<argc;++i)
        {
            if(std::strlen(argv[i])>=2&&argv[i][0]=='-'&&argv[i][1]=='-')
            {
                if(std::strcmp(argv[i],"--no-stage")==0)
                    complete=false;
                else if(std::strcmp(argv[i],"--stage")==0)
                        complete=true;
            }

                else modelName=argv[i];
        }
        mainWindow->loadModel(QDir::fromNativeSeparators(modelName),complete);
    }

    splash.finish(mainWindow);
//    mainWindow->hide(); // get out of the way while user chooses model


    int ret = app.exec();

    delete mainWindow;
    return ret;
}
