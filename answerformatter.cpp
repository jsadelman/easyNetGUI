#include "answerformatter.h"

#include <QDebug>
#include <QDomDocument>
#include <QRegExp>

void XMLFormatter::formatAnswer(QString answer)
{
    QDomDocument *domDoc = new QDomDocument;
    domDoc->setContent(answer); // R.I.P. Bison
    emit formattedAnswer(domDoc);
}


void ListOfValuesFormatter::formatAnswer(QString answer)
{
    // extracts value attributes from the first level of an XML answer
    QStringList list;
    QDomDocument *domDoc = new QDomDocument;
    domDoc->setContent(answer);
    QDomNode objectNode = domDoc->firstChild().firstChild();
    while (!objectNode.isNull())
    {
        if (objectNode.isElement())
            list.append(objectNode.toElement().attribute("value"));
        objectNode = objectNode.nextSibling();
    }
    delete domDoc;
    emit formattedAnswer(list);
}


void SVGFormatter::formatAnswer(QString answer)
{
    emit formattedAnswer(answer.toUtf8());
}
