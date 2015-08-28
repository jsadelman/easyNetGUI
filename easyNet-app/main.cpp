#include "easyNetMainWindow.h"
#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QFont largerFont("Georgia", 16);
//    QFont globalFont("Georgia",
//#ifdef __APPLE__
//                     12
//                 #else
//                     10
//                 #endif
//                     );
//    a.setFont(globalFont);
    EasyNetMainWindow mainWindow;
    mainWindow.showMaximized();

    return a.exec();
}
