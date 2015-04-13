#include "answerformatter.h"

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
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
    if (!domDoc->isNull())
    {
        QDomElement objectElem = domDoc->documentElement().firstChildElement();
        while (!objectElem.isNull())
        {
            list.append(objectElem.attribute("value"));
            objectElem = objectElem.nextSiblingElement();
        }
    }
    delete domDoc;
    emit formattedAnswer(list);
}


void SVGFormatter::formatAnswer(QString answer)
{
    emit formattedAnswer(answer.toUtf8());
}
