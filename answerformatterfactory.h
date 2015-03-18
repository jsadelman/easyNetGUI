#ifndef ANSWERFORMATTERFACTORY_H
#define ANSWERFORMATTERFACTORY_H

#include <QObject>
#include "enumclasses.h"

class AnswerFormatter;

class AnswerFormatterAbstractFactory
{
public:
    virtual AnswerFormatter * newAnswerFormatter(AnswerFormatterType formatterClassName,
                                                 QObject *answerRecipient,
                                                 char const *answerProcessor) = 0;
    virtual ~AnswerFormatterAbstractFactory() {}
};


class AnswerFormatterFactory : public QObject, public AnswerFormatterAbstractFactory
{
    Q_OBJECT
public:
    static AnswerFormatterFactory* instance();
    virtual AnswerFormatter *newAnswerFormatter(AnswerFormatterType formatterName,
                                                QObject *answerRecipient,
                                                char const *answerProcessor);

protected:
    AnswerFormatterFactory(QObject *parent = 0): QObject(parent){}
};


#endif // ANSWERFORMATTERFACTORY_H
