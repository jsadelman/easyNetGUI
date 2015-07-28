#ifndef LAZYNUTJOBPARAM_H
#define LAZYNUTJOBPARAM_H

#include <functional>
#include <QString>
#include <QStringList>

#include "enumclasses.h"

class QObject;

struct LazyNutJobParam
{
    LazyNutJobParam();
    unsigned int logMode;
//    JobOrigin jobOrigin;
    QStringList cmdList;
    std::function<QString (const QString &)> cmdFormatter;
    AnswerFormatterType answerFormatterType;
    void setAnswerReceiver(QObject *receiver, char const *slot)
        {answerReceiver = receiver; answerSlot = slot;}

    void setEndOfJobReceiver(QObject *receiver, char const *slot)
        {endOfJobReceiver = receiver; endOfJobSlot = slot;}

    void setNextJobReceiver(QObject *receiver, char const *slot)
        {nextJobReceiver = receiver; nextJobSlot = slot;}

    QObject *answerReceiver;
    char const *answerSlot;
    QObject *endOfJobReceiver;
    char const *endOfJobSlot;
    QObject *nextJobReceiver;
    char const *nextJobSlot;

};

#endif // LAZYNUTJOBPARAM_H
