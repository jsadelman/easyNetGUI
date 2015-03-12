#include "lazynutjob.h"
#include "lazynutmacro.h"
#include <QSignalTransition>
#include <functional>

LazyNutJob::LazyNutJob(LazyNutMacro *macro) :
    jobOrigin(JobOrigin::User), QState(macro), macro(macro)
{
    addTransition(macro,SIGNAL(next()),macro->endOfMacro); // this --> endOfMacro
}

LazyNutJob::~LazyNutJob()
{
}

void LazyNutJob::setAnswerFormatter(AnswerFormatter *af)
{
    answerFormatter = af;
    af->setParent(this);
    connect(this,&LazyNutJob::exited,[=](){delete answerFormatter;});
}


void LazyNutJob::runCommands()
{
    for (int i = 0; i < cmdList.size(); ++i)
        cmdList[i] = cmdFormatter(cmdList[i]);
    emit runCommands(cmdList, jobOrigin);
}


