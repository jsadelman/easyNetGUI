#include "sessionmanager.h"
#include "commandsequencer.h"
#include "jobqueue.h"
#include "lazynut.h"


#include <QtGlobal>
#include <QFinalState>
#include <QFileInfo>
#include <QBuffer>
#include <QDebug>
#include <QDomDocument>

class MacroQueue;


SessionManager::SessionManager(QObject *parent)
    : QObject(parent),
      lazyNutHeaderBuffer(""), lazyNutOutput(""), OOBrex("OOB secret: (\\w+)\\n")
{
    lazyNut = new LazyNut(this);
    macroQueue = new MacroQueue;

    startCommandSequencer();
}


void SessionManager::startLazyNut(QString lazyNutBat)
{
    connect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(getOOB(QString)));
    lazyNut->setWorkingDirectory(QFileInfo(lazyNutBat).absolutePath());
    lazyNut->start(lazyNutBat);
    // TODO: here we should have a wait until timeout, since lazyNut could be on a remote server
    if (lazyNut->state() == QProcess::NotRunning)
    {
        delete lazyNut;
        emit lazyNutNotRunning();
    }
}

void SessionManager::getOOB(const QString &lazyNutOutput)
{
    lazyNutHeaderBuffer.append(lazyNutOutput);
    if (lazyNutHeaderBuffer.contains(OOBrex))
    {
        OOBsecret = OOBrex.cap(1);
        qDebug() << OOBsecret;
        lazyNutHeaderBuffer.clear();
        disconnect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(getOOB(QString)));
    }
}

void SessionManager::startCommandSequencer()
{
    commandSequencer = new CommandSequencer(lazyNut, this);
    connect(commandSequencer,SIGNAL(userLazyNutOutputReady(QString)),
            this,SIGNAL(userLazyNutOutputReady(QString)));
}


void SessionManager::updateRecentlyModified(QDomDocument*dom)
{
    QStringList _recentlyModified=extrctRecentlyModifiedList(dom);
    recentlyModified.append(_recentlyModified);
}


void SessionManager::killLazyNut()
{
    lazyNut->kill();
}


void SessionManager::runCommands()
{
    commandSequencer->runCommands(commandList, JobOrigin::User,commandSequencer,false,SIGNAL(null()));
}

QStateMachine *SessionManager::buildMacro()
{
    QStateMachine * macro = new QStateMachine(this);
    connect(macro,SIGNAL(started()),this,SLOT(macroStarted()));
    connect(macro,SIGNAL(finished()),this,SLOT(macroEnded()));
    connect(macro,SIGNAL(finished()),macro,SLOT(deleteLater()));
    return macro;
}


void SessionManager::runModel(QStringList cmdList)
{
    commandList = cmdList;
    QStateMachine * macro = buildMacro();
    // states
    QState * runCommandsState = new QState(macro);
    connect(runCommandsState,SIGNAL(entered()),this,SLOT(runCommands()));
    QState * getSubtypesState = new QState(macro);
    connect(getSubtypesState,SIGNAL(entered()),this,SLOT(getSubtypes()));
    QState * getRecentlyModifiedState = new QState(macro);
    connect(getRecentlyModifiedState,SIGNAL(entered()),this,SLOT(getRecentlyModified()));
    QState * getDescriptionsState = new QState(macro);
    connect(getDescriptionsState,SIGNAL(entered()),this,SLOT(getDescriptions()));
    connect(getDescriptionsState,SIGNAL(exited()),this,SIGNAL(updateDiagramScene()));

    QFinalState *finalState = new QFinalState(macro);
    macro->setInitialState(runCommandsState);

    // transitions
    runCommandsState->addTransition(commandSequencer,SIGNAL(commandsExecuted()),getSubtypesState);
    getSubtypesState->addTransition(commandSequencer,SIGNAL(commandsExecuted()),getRecentlyModifiedState);
    getRecentlyModifiedState->addTransition(commandSequencer,SIGNAL(commandsExecuted()),getDescriptionsState);
    getDescriptionsState->addTransition(commandSequencer,SIGNAL(commandsExecuted()),finalState);
    getDescriptionsState->addTransition(this,SIGNAL(skipDescriptions()),finalState);

    macroQueue->tryRun(macro);
}

void SessionManager::runSelection(QStringList cmdList)
{
    commandList = cmdList;
    QStateMachine * macro = buildMacro();
    // states
    QState * runCommandsState = new QState(macro);
    connect(runCommandsState,SIGNAL(entered()),this,SLOT(runCommands()));
    QState * getRecentlyModifiedState = new QState(macro);
    connect(getRecentlyModifiedState,SIGNAL(entered()),this,SLOT(getRecentlyModified()));
    QState * getDescriptionsState = new QState(macro);
    connect(getDescriptionsState,SIGNAL(entered()),this,SLOT(getDescriptions()));
    connect(getDescriptionsState,SIGNAL(exited()),this,SIGNAL(updateDiagramScene()));
    QFinalState *finalState = new QFinalState(macro);
    macro->setInitialState(runCommandsState);

    // transitions
    runCommandsState->addTransition(commandSequencer,SIGNAL(commandsExecuted()),getRecentlyModifiedState);
    getRecentlyModifiedState->addTransition(commandSequencer,SIGNAL(commandsExecuted()),getDescriptionsState);
    getDescriptionsState->addTransition(commandSequencer,SIGNAL(commandsExecuted()),finalState);
    getDescriptionsState->addTransition(this,SIGNAL(skipDescriptions()),finalState);

    macroQueue->tryRun(macro);
}


bool SessionManager::getStatus()
{
    return commandSequencer->getStatus();
}

void SessionManager::pause()
{
    macroQueue->pause();
//    commandSequencer->pause();
    emit isPaused(macroQueue->isPaused());
}

void SessionManager::stop()
{
    macroQueue->stop();
//    commandSequencer->stop();
}


void SessionManager::getSubtypes()
{
    commandList.clear();
    foreach (QString type, lazyNutObjTypes)
        commandList.append(QString("xml subtypes %1").arg(type));
    commandSequencer->runCommands(commandList, JobOrigin::GUI,this,false,SLOT(null()));
}

void SessionManager::getRecentlyModified()
{
    commandList.clear();
    commandList << "xml recently_modified";
    commandSequencer->runCommands(commandList, JobOrigin::GUI,this,true,SLOT(updateRecentlyModified(QDomDocument*)));
    commandList.clear();
    commandList << "clear_recently_modified";
    commandSequencer->runCommands(commandList, JobOrigin::GUI,this,false,SLOT(null()));
}

QStringList SessionManager::extrctRecentlyModifiedList(QDomDocument *domDoc)
{
    QStringList recentlyModified;
    QDomNode objectNode = domDoc->firstChild().firstChild();
    while (!objectNode.isNull())
    {
        if (objectNode.nodeName() == "object")
            recentlyModified.append(objectNode.toElement().attribute("value"));
        objectNode = objectNode.nextSibling();
    }
    return recentlyModified;
}

void SessionManager::clearRecentlyModified()
{
    commandList.clear();
    commandList << "clear_recently_modified";
    commandSequencer->runCommands(commandList, JobOrigin::GUI, this, false, SLOT());
}

void SessionManager::getDescriptions()
{
    if (recentlyModified.isEmpty())
        emit skipDescriptions();
    else
    {
        commandList.clear(); // really?
        foreach (QString obj, recentlyModified)
        {
            commandSequencer->runCommand(QString("xml %1 description").arg(obj), JobOrigin::GUI, this, true, SIGNAL(descriptionReady(QDomDocument *)));
        }
        recentlyModified.clear();
    }
}

void SessionManager::macroStarted()
{
    qDebug() << "Macro started.";
}

void SessionManager::macroEnded()
{
    qDebug() << "Macro ended.";
    macroQueue->freeToRun();
}



void MacroQueue::run(QStateMachine *macro)
{
    qDebug() << "Jobs in MacroQueue:" << queue.size();
    macro->start();
}

void MacroQueue::reset()
{
    qDebug() << "RESET CALLED, Jobs in MacroQueue:" << queue.size();
    while (!queue.isEmpty())
    {
        delete queue.dequeue();
    }
    if (currentJob)
        currentJob->stop();
    //delete currentJob;
}

QString MacroQueue::name()
{
    return "MacroQueue";
}
