#ifndef LAZYNUTJOB_H
#define LAZYNUTJOB_H

#include "enumclasses.h"

#include <QObject>
#include <QStringList>

class MacroQueue;
class AnswerFormatter;

class LazyNutJob : public QObject
{
    Q_OBJECT
public:
    explicit LazyNutJob(MacroQueue* queue);
    ~LazyNutJob();
    bool active() {return m_active;}
    unsigned int logMode;
    QStringList cmdList;
    void setAnswerReceiver(QObject *receiver, char const *slot, AnswerFormatterType answerFormatterType);
    void setErrorReceiver(QObject *receiver, char const *slot);
    void setEndOfJobReceiver(QObject *receiver, char const *slot);

public slots:
    void run();

signals:
    void runCommands(QStringList, bool, unsigned int);
    void cmdError(QString, QStringList);
    void finished();

private slots:
    void finish();
    void formatAnswer(QString answer, QString cmd);
    void sendCmdError(QString cmd, QStringList errorList);


private:
    void setActive(bool isActive) {m_active = isActive;}

    bool m_active;
    MacroQueue* queue;
    AnswerFormatter *answerFormatter;
};

#endif // LAZYNUTJOB_H
