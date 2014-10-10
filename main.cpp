#include "nmConsole.h"
#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont largerFont("Courier", 12);
    a.setFont(largerFont);
    NmConsole w;
    w.show();

    return a.exec();
}
