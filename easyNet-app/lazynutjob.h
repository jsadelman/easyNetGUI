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
    void setCmdList(QString cmd) {cmdList = QStringList({cmd});}
//    void setJobOrigin(JobOrigin origin) {jobOrigin = origin;}

    LazyNutMacro *macro;
    QStringList cmdList;
    AnswerFormatter *answerFormatter;
    std::function<QString (const QString& s)> cmdFormatter;
    bool echoOnInterpreter;
    unsigned int logMode;

signals:
    void runCommands(QStringList, bool, unsigned int);
    void cmdError(QString, QStringList);


public slots:
    void runCommands();


private slots:
    void formatAnswer(QString answer, QString cmd);

};

#endif // LAZYNUTJOB_H
