#ifndef COMMANDSEQUENCER_H
#define COMMANDSEQUENCER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QQueue>
#include <QSet>
#include <QVector>
//#include <QProcess>

#include "enumclasses.h"

class LazyNut;
class QDomDocument;


class CommandSequencer: public QObject
{
    Q_OBJECT

public:
    CommandSequencer(LazyNut* lazyNut, QObject *parent=0);


public slots:
    void runCommands(QStringList commands, bool _getAnswer, unsigned int mode);
    void runCommand(QString command, bool _getAnswer, unsigned int mode);
    // status
    bool getStatus();
    bool isOn();

    void processLazyNutOutput(const QString &lazyNutOutput);

signals:
    // send output to editor
    void commandsInJob(int);
    void commandsExecuted();
    void commandExecuted(QString,QString);
    void logCommand(QString);
    void commandSent(QString);
    void answerReady(QString, QString);
    void userLazyNutOutputReady(const QString&);
    // states
    void isReady(bool);
    // errors
    void cmdError(QString,QStringList);
    void cmdR(QString,QStringList);

    void dotsCount(int);

private:

    void initProcessLazyNutOutput();

    bool getAnswer;
    unsigned int logMode;
    bool ready;
    bool on;
    LazyNut* lazyNut;
    QStringList commandList;
    QString lazyNutBuffer;
    int baseOffset;
    QRegExp beginRex;
    QRegExp emptyLineRex;
    QRegExp errorRex,rRex;
    QRegExp answerRex;
    QRegExp dotsRex;
    QRegExp eNelementsRex;
    QRegExp xmlStartRex;
    QRegExp svgRex;
    QRegExp answerDoneRex;

};

#endif // COMMANDSEQUENCER_H
