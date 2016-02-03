#include "trialruninfo.h"
#include "xmlaccessor.h"

TrialRunInfo::TrialRunInfo(QSharedPointer<QDomDocument> info)
    : info(info)
{
}

QString TrialRunInfo::getField(QString field)
{
    QDomElement rootElement = info->documentElement();
    QDomElement fieldElem = XMLAccessor::childElement(rootElement, field);
    return XMLAccessor::value(fieldElem);
}
