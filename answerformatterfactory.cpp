#include "answerformatterfactory.h"
#include "answerformatter.h"
#include <QDebug>
#include <QApplication>
#include <QDomDocument>


AnswerFormatterFactory *AnswerFormatterFactory::instance()
{
    static AnswerFormatterFactory * singleton = nullptr;
    if (singleton == nullptr)
    {
        singleton = new AnswerFormatterFactory(qApp);
    }
    return singleton;
}

AnswerFormatter *AnswerFormatterFactory::newAnswerFormatter(AnswerFormatterType formatterName, QObject *answerRecipient, char const *answerProcessor)
{
    AnswerFormatter *af;
    if (formatterName == AnswerFormatterType::NotInitialised)
        af = nullptr;
    else if (formatterName == AnswerFormatterType::Identity)
        af = new IdentityFormatter(answerRecipient, answerProcessor);
    else if (formatterName == AnswerFormatterType::XML)
        af = new XMLFormatter(answerRecipient, answerProcessor);
    else if (formatterName == AnswerFormatterType::ListOfValues)
        af = new ListOfValuesFormatter(answerRecipient, answerProcessor);
    else if (formatterName == AnswerFormatterType::SVG)
        af = new SVGFormatter(answerRecipient, answerProcessor);
    return af;
}

