#include "easyNetMainWindow.h"
#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont largerFont("Georgia", 16);
    QFont globalFont("Georgia", 10);
    a.setFont(globalFont);
    EasyNetMainWindow mainWindow;
    mainWindow.showMaximized();

    return a.exec();
}
