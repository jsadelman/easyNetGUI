#include "lazynut.h"
#include <QDebug>

LazyNut::LazyNut(QObject *parent)
{
    connect(this,SIGNAL(readyReadStandardError()),this,SLOT(getOutput()));
}

LazyNut::~LazyNut()
{
    terminate();
}

void LazyNut::sendCommand(QString command)
{
    write(qPrintable(command + "\n"));
}

void LazyNut::getOutput()
{
    //qDebug () << "LazyNut::getOutput()";
    emit outputReady(QString(readAllStandardError()));
}
