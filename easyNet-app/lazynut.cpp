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
    QByteArray ba = readAllStandardError();
    QString st = QString(ba);
    emit outputReady(st);
//    emit outputReady(QString(readAllStandardError()));
}