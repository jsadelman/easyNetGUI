#include "lazynutmacro.h"
#include "lazynutjob.h"
#include <QDebug>


LazyNutMacro::LazyNutMacro(QObject *parent) :
    QStateMachine(parent)
{
    endOfMacro = new QFinalState(this);
    connect(this,SIGNAL(started()),this,SLOT(macroStarted()));
    connect(this,SIGNAL(finished()),this,SLOT(macroEnded()));
    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}


void LazyNutMacro::macroStarted()
{
     qDebug() << "macroStarted";
}


void LazyNutMacro::macroEnded()
{
    qDebug() << "macroEnded";
}
