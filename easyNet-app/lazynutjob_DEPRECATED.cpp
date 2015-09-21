#include "lazynutjob_DEPRECATED.h"
#include "lazynutmacro.h"
#include <QSignalTransition>
#include <functional>

LazyNutJob_DEPRECATED::LazyNutJob_DEPRECATED(LazyNutMacro *macro) :
    QState(macro), macro(macro), answerFormatter(nullptr)
{
    addTransition(macro,SIGNAL(next()),macro->endOfMacro); // this --> endOfMacro
}

LazyNutJob_DEPRECATED::~LazyNutJob_DEPRECATED()
{
}

void LazyNutJob_DEPRECATED::setAnswerFormatter(AnswerFormatter *af)
{
    answerFormatter = af;
    af->setParent(this);
    connect(this,&LazyNutJob_DEPRECATED::exited,[=](){delete answerFormatter;});
}


void LazyNutJob_DEPRECATED::runCommands()
{
    for (int i = 0; i < cmdList.size(); ++i)
        cmdList[i] = cmdFormatter(cmdList[i]);
    emit runCommands(cmdList, answerFormatter, logMode);
}

void LazyNutJob_DEPRECATED::formatAnswer(QString answer, QString cmd)
{
    if (active() && answerFormatter)
        answerFormatter->formatAnswer(answer, cmd);

}

