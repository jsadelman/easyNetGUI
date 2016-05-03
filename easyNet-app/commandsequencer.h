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
    bool echoInterpreter(QString cmd);


public slots:
    void runCommands(QStringList commands, bool _getAnswer, unsigned int mode);
    void runCommand(QString command, bool _getAnswer, unsigned int mode);
    // status
    bool getStatus();
    bool isOn();
    void clearCommandList();

    void processLazyNutOutput(QString lazyNutOutput);

signals:
    // send output to editor
    void commandsInJob(int);
    void jobExecuted();
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
//    QString lazyNutBuffer;
    QStringList lazyNutLines;
    QString lazyNutIncompleteLine;
    int baseOffset;
    QRegExp emptyLineRex;

    void processLazyNutLine();
    int dotcount;
    int beginLine;
    QString currentAnswer,currentCmd;
    bool timeMode,svgMode;
    int bytesPending;
};

#endif // COMMANDSEQUENCER_H
