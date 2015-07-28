#ifndef MACROQUEUE_H
#define MACROQUEUE_H

#include <QString>
#include <QStateMachine>
#include "jobqueue.h"


class MacroQueue: public JobQueue<QStateMachine,MacroQueue>
{
public:
    MacroQueue(): JobQueue(){}
    void run(QStateMachine *macro);
    void reset();
    QString name();
};

#endif // MACROQUEUE_H
