#ifndef MACROQUEUE_H
#define MACROQUEUE_H

#include <QString>
#include <QStateMachine>
#include "jobqueue_DEPRECATED.h"


class MacroQueue: public JobQueue_DEPRECATED<QStateMachine,MacroQueue>
{
public:
    MacroQueue(): JobQueue_DEPRECATED(){}
    void run(QStateMachine *macro);
    void reset();
    QString name();
};

#endif // MACROQUEUE_H
