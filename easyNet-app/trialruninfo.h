#ifndef TRIALRUNINFO_H
#define TRIALRUNINFO_H

#include <QSharedPointer>

class QDomDocument;

class TrialRunInfo
{
public:
    TrialRunInfo(QSharedPointer<QDomDocument> info);
    QString results()   {return getField("Results");}
    QString trial()     {return getField("Trial");}
    QString runMode()   {return getField("Run mode");}

private:
    QSharedPointer<QDomDocument> info;
    QString getField(QString field);
};

#endif // TRIALRUNINFO_H
