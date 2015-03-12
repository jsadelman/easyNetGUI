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
    JobOrigin jobOrigin;
    QStringList cmdList;
    std::function<QString (const QString &)> cmdFormatter;
    QString answerFormatterType;
    QObject *answerReceiver;
    char const *answerSlot;
    QObject *finalReceiver;
    char const *finalSlot;
    QObject *nextJobReceiver;
    char const *nextJobSlot;
};

#endif // LAZYNUTJOBPARAM_H
