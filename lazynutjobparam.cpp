#include "lazynutjobparam.h"

LazyNutJobParam::LazyNutJobParam()
    : logMode(0), cmdList({}), cmdFormatter([] (QString cmd) {return cmd;}),
    answerFormatterType(AnswerFormatterType::NotInitialised), answerReceiver(nullptr), answerSlot(nullptr),
    endOfJobReceiver(nullptr), endOfJobSlot(nullptr), nextJobReceiver(nullptr), nextJobSlot(nullptr)
{
}
