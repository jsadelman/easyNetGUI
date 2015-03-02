#ifndef COMMANDSEQUENCER_H
#define COMMANDSEQUENCER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QQueue>
#include <QSet>
#include <QVector>
#include <QDomDocument>
#include <QDebug>

#include "enumclasses.h"

class LazyNut;
class QDomDocument;

class LazyNutCommand: public QObject
{
    Q_OBJECT

public:
    operator QString()const
    {
        return cmdText;
    }

    LazyNutCommand(QString x):cmdText(x),object(0),xmlAfter(false){}

    LazyNutCommand(QString x,QObject*obj,char const* sl,bool xml):
        cmdText(x),object(obj),xmlAfter(xml)
    {
        if(xml)
        {
            connect(this,SIGNAL(answer(QDomDocument*)),obj,sl);
        }
        else
        {
            connect(this,SIGNAL(answer(QString)),obj,sl);
        }
    }
private:
    QString cmdText;
    QObject* object;
    bool xmlAfter;

public:
    void done(const QString&ans)
    {
       if(xmlAfter)
       {
           QDomDocument *domDoc = new QDomDocument;
           domDoc->setContent(ans); // this line replaces an entire Bison!
           emit answer(domDoc);
       }
       else
       {
           emit answer(ans);
       }
    }

signals:
    void answer(const QString&ans);
    void answer(QDomDocument*dom);
};

class CommandSequencer: public QObject
{
    Q_OBJECT

public:

    CommandSequencer(LazyNut* lazyNut, QObject *parent=0);


public:
    void runCommands(QStringList commands, JobOrigin origin, QObject* obj, bool xml, char const* slot);
    void runCommand(QString command, JobOrigin origin, QObject*, bool xml, char const* slot);
    // status
public slots:
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

    JobOrigin jobOrigin;
    bool ready;
    LazyNut* lazyNut;
    QList<LazyNutCommand*> commandList;
    QString lazyNutBuffer;
    int baseOffset;
    QRegExp emptyLineRex;
    QRegExp errorRex;
    QRegExp answerRex;
    QVector<QString> xmlCmdTags;
    QRegExp eNelementsTagRex;
//    QSet<LazyNutCommandTypes> queryTypes;

};

#endif // COMMANDSEQUENCER_H
