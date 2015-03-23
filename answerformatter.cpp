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
    QRegExp answerRex = QRegExp("SVG file of (\\d+) bytes:\\s?\\n");
    int pos = answerRex.indexIn(answer);
    int size;
    QString leftOver;
    if (pos > -1)
    {
         size = answerRex.cap(1).toInt();
         leftOver = answer.remove(0, answerRex.matchedLength());
         emit formattedAnswer(size, leftOver.toLatin1());
         qDebug() << "Found " << size <<" bytes in" << answer.left(30);
    }
    else
        qDebug() << "Failed to find # bytes in" << answer.left(30);
}
