#ifndef LAZYNUTJOB_H
#define LAZYNUTJOB_H

#include <QState>
#include <functional>
#include <QStringList>
#include "enumclasses.h"

#include "answerformatter.h"

class LazyNutMacro;


class LazyNutJob : public QState
{
    Q_OBJECT
public:
    explicit LazyNutJob(LazyNutMacro *macro);
    ~LazyNutJob();
    void setAnswerFormatter(AnswerFormatter* af);
    void setCmdFormatter(std::function<QString (const QString& s)> cf) {cmdFormatter = cf;}
    void setCmdList(QStringList list) {cmdList = list;}
    void setCmdList(QString cmd) {cmdList = {cmd};}
    void setJobOrigin(JobOrigin origin) {jobOrigin = origin;}

    LazyNutMacro *macro;
    QStringList cmdList;
    AnswerFormatter *answerFormatter;
    std::function<QString (const QString& s)> cmdFormatter;
    JobOrigin jobOrigin;

signals:
    void runCommands(QStringList, JobOrigin);

public slots:
    void runCommands();

};

#endif // LAZYNUTJOB_H
