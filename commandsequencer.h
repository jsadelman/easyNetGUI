#ifndef COMMANDSEQUENCER_H
#define COMMANDSEQUENCER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QQueue>
#include <QSet>
#include <QVector>

#include "enumclasses.h"

class LazyNut;
class QDomDocument;


class CommandSequencer: public QObject
{
    Q_OBJECT

public:


    CommandSequencer(LazyNut* lazyNut, QObject *parent=0);


public slots:
    void runCommands(QStringList commands, JobOrigin origin);
    void runCommand(QString command, JobOrigin origin);
    // status
    bool getStatus();

    void processLazyNutOutput(const QString &lazyNutOutput);
signals:
    // send output to editor
    void commandsExecuted();
    void answerReady(QString);
    void userLazyNutOutputReady(const QString&);
    // states
    void isReady(bool);
    // errors
    void cmdError(QString,QStringList);

private:

    void initProcessLazyNutOutput();

    JobOrigin jobOrigin;
    bool ready;
    LazyNut* lazyNut;
    QStringList commandList;
    QString lazyNutBuffer;
    int baseOffset;
    QRegExp emptyLineRex;
    QRegExp errorRex;
    QRegExp answerRex;

};

#endif // COMMANDSEQUENCER_H
