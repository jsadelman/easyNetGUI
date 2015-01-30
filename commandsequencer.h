#ifndef COMMANDSEQUENCER_H
#define COMMANDSEQUENCER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QQueue>

#include "enumclasses.h"

class LazyNut;


class CommandSequencer: public QObject
{
    Q_OBJECT

public:
    CommandSequencer(LazyNut* lazyNut, QObject *parent=0);


public slots:
    void runCommands(QStringList commands,
                     JobOrigin origin);
    void runCommand(QString command,
                    JobOrigin origin);
    void receiveLazyNutOutput(const QString &lazyNutOutput);
    // status
    bool getStatus();

signals:
    // send output to editor
    void commandsExecuted(QString,JobOrigin);
    void userLazyNutOutputReady(const QString&);
    // states
    void isReady(bool);
    // errors
    void cmdError(QString,QStringList);

private:

    JobOrigin jobOrigin;
    bool ready;
    LazyNut* lazyNut;
    QStringList commandList;
    QString lazyNutBuffer;
    int baseOffset;
    QRegExp emptyLineRex;

};

#endif // COMMANDSEQUENCER_H
