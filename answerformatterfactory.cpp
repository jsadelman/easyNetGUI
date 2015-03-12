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

AnswerFormatter *AnswerFormatterFactory::newAnswerFormatter(QString formatterName, QObject *answerRecipient, char const *answerProcessor)
{
    AnswerFormatter *af;
    if (formatterName == "Identity")
        af = new IdentityFormatter(answerRecipient, answerProcessor);
    else if (formatterName == "XML")
        af = new XMLFormatter(answerRecipient, answerProcessor);
    else if (formatterName == "ListOfValues")
        af = new ListOfValuesFormatter(answerRecipient, answerProcessor);
    else
    {
        qDebug() << "Unknown AnswerFormatter class provided. Returing a null pointer.";
        af = nullptr;
    }
    return af;
}

