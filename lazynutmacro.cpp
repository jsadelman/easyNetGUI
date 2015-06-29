#include "lazynutmacro.h"
#include "lazynutjob.h"
#include "macroqueue.h"
#include <QDebug>


LazyNutMacro::LazyNutMacro(MacroQueue *queue, QObject *parent) :
    macroQueue(queue), QStateMachine(parent)
{
    endOfMacro = new QFinalState(this);
    connect(this,SIGNAL(started()),this,SLOT(macroStarted()));
    connect(this,SIGNAL(finished()),this,SLOT(macroEnded()));
    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}


void LazyNutMacro::macroStarted()
{
//     qDebug() << "macroStarted";
}


void LazyNutMacro::macroEnded()
{
    macroQueue->freeToRun();
//    qDebug() << "macroEnded";
}
