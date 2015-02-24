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
    enum LazyNutCommandTypes
    {
        description,
        recently_modified,
        subtypes,
        LazyNutCommandTypes_MAX = subtypes
    };

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
    void queryAnswersReady(QString);
    void userLazyNutOutputReady(const QString&);

    void recentlyModifiedReady(QStringList);
    void descriptionReady(QDomDocument*);
    // states
    void isReady(bool);
    // errors
    void cmdError(QString,QStringList);

private:

    void initProcessLazyNutOutput();
    QStringList extrctRecentlyModifiedList(QDomDocument* domDoc);

    JobOrigin jobOrigin;
    bool ready;
    LazyNut* lazyNut;
    QStringList commandList;
    QString lazyNutBuffer;
    int baseOffset;
    QRegExp emptyLineRex;
    QRegExp errorRex;
    QRegExp answerRex;
    QVector<QString> xmlCmdTags;
    QRegExp eNelementsTagRex;
    QSet<LazyNutCommandTypes> queryTypes;

};

#endif // COMMANDSEQUENCER_H
