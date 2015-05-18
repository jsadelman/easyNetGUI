#include "macroqueue.h"


void MacroQueue::run(QStateMachine *macro)
{
    qDebug() << "Jobs in MacroQueue:" << queue.size();
    macro->start();
}

void MacroQueue::reset()
{
    qDebug() << "RESET CALLED, Jobs in MacroQueue:" << queue.size();
    while (!queue.isEmpty())
    {
        delete queue.dequeue();
    }
    if (currentJob)
        currentJob->stop();
    //delete currentJob;
}

QString MacroQueue::name()
{
    return "MacroQueue";
}
