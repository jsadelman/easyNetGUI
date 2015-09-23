#ifndef LAZYNUTJOB_H
#define LAZYNUTJOB_H

#include "enumclasses.h"

#include <QObject>
#include <QStringList>
#include <QVariant>

template <class Job>
class JobQueue;
class LazyNutJob;
typedef JobQueue<LazyNutJob*> LazyNutJobQueue;

class AnswerFormatter;


class LazyNutJob : public QObject
{
    Q_OBJECT
public:
    explicit LazyNutJob();
    ~LazyNutJob();
    bool active() {return m_active;}
    void setAnswerReceiver(QObject *receiver, char const *slot, AnswerFormatterType answerFormatterType);
    void appendErrorReceiver(QObject *receiver, char const *slot)
        {errorReceiverList.append(receiver); errorSlotList.append(slot);}
    void appendEndOfJobReceiver(QObject *receiver, char const *slot)
        {endOfJobReceiverList.append(receiver); endOfJobSlotList.append(slot);}

    unsigned int logMode;
    QStringList cmdList;
    QVariant data;

public slots:
    void run();

signals:
//    void runCommands(QStringList, bool, unsigned int);
    void cmdError(QString, QStringList);
    void finished();

private slots:
    void finish();
    void formatAnswer(QString answer, QString cmd);
    void sendCmdError(QString cmd, QStringList errorList);


private:
    void setActive(bool isActive) {m_active = isActive;}

    bool m_active;
    AnswerFormatter *answerFormatter;
    QList<QObject *> errorReceiverList;
    QList<char const *> errorSlotList;
    QList<QObject *> endOfJobReceiverList;
    QList<char const *> endOfJobSlotList;
//    QObject *errorReceiver;
//    char const *errorSlot;
//    QObject *endOfJobReceiver;
//    char const *endOfJobSlot;
};

#endif // LAZYNUTJOB_H
