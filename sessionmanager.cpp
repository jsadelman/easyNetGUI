#include "sessionmanager.h"
#include "commandsequencer.h"
#include "jobqueue.h"
#include "treeitem.h"
#include "treemodel.h"
#include "lazynutobj.h"
#include "querycontext.h"
#include "driver.h"



#include <QtGlobal>
#include <QFinalState>
#include <QDebug>

class MacroQueue;
typedef QHash<QString,LazyNutObj*> LazyNutObjCatalogue;

class LazyNut;


SessionManager::SessionManager(LazyNut* lazyNut, LazyNutObjCatalogue* objCatalogue, TreeModel* objTaxonomyModel, QObject *parent)
    : lazyNut(lazyNut), objCatalogue(objCatalogue), objTaxonomyModel(objTaxonomyModel), QObject(parent),
      synchMode(SynchMode::Asynch), lazyNutBuffer(""), rxEND("END:[^\\n]*\\n")
{
    context = new QueryContext;
    driver = new lazyNutOutput::Driver(*context);
    commandSequencer = new CommandSequencer(lazyNut, this);
    macroQueue = new MacroQueue;

    connect(commandSequencer,SIGNAL(currentReceivedCount(int)),
            this,SIGNAL(currentReceivedCount(int)));
    connect(commandSequencer,SIGNAL(isReady(bool)),
            this,SIGNAL(isReady(bool)));
    connect(this,SIGNAL(commandExecuted(QString)),
            commandSequencer,SLOT(receiveResult(QString)));
}

void SessionManager::parseLazyNutOutput(const QString &lazyNutOutput)
{
    lazyNutBuffer.append(lazyNutOutput);
    int indexInLazyNutBuffer = rxEND.indexIn(lazyNutBuffer);
    int lengthRemainder;
    while (indexInLazyNutBuffer >=0)
    {
        lengthRemainder = lazyNutBuffer.size() - indexInLazyNutBuffer - rxEND.matchedLength();
        QString remainder = lazyNutBuffer.right(lengthRemainder);
        lazyNutBuffer.chop(lengthRemainder);
        bool parseSuccessful = driver->parse_string(lazyNutBuffer.toStdString(), "lazyNutOutput");
        if (parseSuccessful)
            emit commandExecuted(lazyNutBuffer);
        else
            qDebug() << lazyNutBuffer;
        lazyNutBuffer = remainder;
        indexInLazyNutBuffer = rxEND.indexIn(lazyNutBuffer);
    }
}

void SessionManager::processLazyNutOutput()
{
    bool objHashModified = false;
    foreach (TreeItem* queryItem, context->root->children())
    {
        QString queryType = queryItem->data(0).toString();
        if (queryType == "subtypes")
        {
            QString objectType = queryItem->data(1).toString();
            QModelIndex objectIndex = objTaxonomyModel->index(0,0);
            int row = 0;
            while (objTaxonomyModel->data(objTaxonomyModel->index(row,0,objectIndex),Qt::DisplayRole).toString() != objectType &&
                   row < objTaxonomyModel->rowCount(objectIndex))
                ++row;
            QModelIndex typeIndex = objTaxonomyModel->index(row,0,objectIndex);
            if (typeIndex.isValid())
            {
                foreach (TreeItem* subtypeItem, queryItem->children())
                {
                    objTaxonomyModel->appendValue(subtypeItem->data(1).toString(),typeIndex);
                }
            }
        }
        else if (queryType == "recently_modified")
        {
            foreach (TreeItem* objectItem, queryItem->children())
            {
                recentlyModified.append(objectItem->data(1).toString());
            }
        }
        else if (queryType == "description")
        {
            emit beginObjHashModified();
            foreach (TreeItem* objectItem, queryItem->children())
            {
                QString objectName = objectItem->data(1).toString();
                objCatalogue->insert(objectName,new LazyNutObj());
                foreach (TreeItem* propertyItem, objectItem->children())
                {
                    QString propertyKey = propertyItem->data(0).toString();
                    QString propertyValue = propertyItem->data(1).toString();
                    if (propertyValue.startsWith('[') && propertyValue.endsWith(']'))
                        // todo: generate query list
                        (*objCatalogue)[objectName]->appendProperty(propertyKey,propertyValue);
                    else if (propertyValue.contains(','))
                        (*objCatalogue)[objectName]->appendProperty(propertyKey,propertyValue.split(", "));
                    else
                        (*objCatalogue)[objectName]->appendProperty(propertyKey,propertyValue);
                }
            }
            objHashModified = true;
        }
    }
    if (objHashModified)
        emit endObjHashModified();
    context->clearQueries();
}



void SessionManager::runCommands(JobOrigin jobOrigin)
{
    commandSequencer->runCommands(commandList, jobOrigin, synchMode);
}

Macro *SessionManager::buildMacro()
{
    Macro * macro = new Macro(this);
    connect(macro,SIGNAL(started()),this,SLOT(macroStarted()));
    connect(macro,SIGNAL(finished()),this,SLOT(macroEnded()));
    connect(macro,SIGNAL(finished()),macro,SLOT(deleteLater()));
    return macro;
}

QueryState *SessionManager::buildQueryState(Macro *macro)
{
    QueryState * queryState = new QueryState(macro);
    connect(queryState,SIGNAL(exited()),this,SLOT(processLazyNutOutput()));
    return queryState;
}


void SessionManager::runModel(QStringList cmdList)
{
    commandList = cmdList;
    Macro * macro = buildMacro();
    // states
    UserState * runCommandsState = new UserState(macro);
    connect(runCommandsState,SIGNAL(enteredWithJobOrigin(JobOrigin)),this,SLOT(runCommands(JobOrigin)));
    QueryState * getSubtypesState = buildQueryState(macro);
    connect(getSubtypesState,SIGNAL(enteredWithJobOrigin(JobOrigin)),this,SLOT(getSubtypes(JobOrigin)));
    QueryState * getRecentlyModifiedState = buildQueryState(macro);
    connect(getRecentlyModifiedState,SIGNAL(enteredWithJobOrigin(JobOrigin)),this,SLOT(getRecentlyModified(JobOrigin)));

    QueryState * getDescriptionsState = buildQueryState(macro);
    connect(getDescriptionsState,SIGNAL(enteredWithJobOrigin(JobOrigin)),this,SLOT(getDescriptions(JobOrigin)));
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
    Macro * macro = buildMacro();
    // states
    UserState * runCommandsState = new UserState(macro);
    connect(runCommandsState,SIGNAL(enteredWithJobOrigin(JobOrigin)),this,SLOT(runCommands(JobOrigin)));
    QueryState * getRecentlyModifiedState = buildQueryState(macro);
    connect(getRecentlyModifiedState,SIGNAL(enteredWithJobOrigin(JobOrigin)),this,SLOT(getRecentlyModified(JobOrigin)));

    QueryState * getDescriptionsState = buildQueryState(macro);
    connect(getDescriptionsState,SIGNAL(enteredWithJobOrigin(JobOrigin)),this,SLOT(getDescriptions(JobOrigin)));
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

int SessionManager::getCurrentReceivedCount()
{
    return commandSequencer->getCurrentReceivedCount();
}

void SessionManager::pause()
{
    macroQueue->pause();
    commandSequencer->pause();
    emit isPaused(macroQueue->isPaused() || commandSequencer->isPaused());
}

void SessionManager::stop()
{
    macroQueue->stop();
    commandSequencer->stop();
}

void SessionManager:: getSubtypes(JobOrigin jobOrigin)
{
    commandList.clear();
    foreach (QString type, lazyNutObjTypes)
        commandList.append(QString("query 1 subtypes %1").arg(type));
    commandSequencer->runCommands(commandList, jobOrigin, synchMode);
}

void SessionManager::getRecentlyModified(JobOrigin jobOrigin)
{
    commandList.clear();
    commandList << "query 1 recently_modified"
                << "query 1 clear_recently_modified";
    commandSequencer->runCommands(commandList, jobOrigin, synchMode);
}

void SessionManager::clearRecentlyModified(JobOrigin jobOrigin)
{
    commandList.clear();
    commandList << "query 1 clear_recently_modified";
    commandSequencer->runCommands(commandList, jobOrigin, synchMode);
}

void SessionManager::getDescriptions(JobOrigin jobOrigin)
{
    if (recentlyModified.isEmpty())
        emit skipDescriptions();
    else
    {
        commandList.clear();
        foreach (QString obj, recentlyModified)
            commandList.append(QString("query 1 %1 description").arg(obj));
        commandSequencer->runCommands(commandList, jobOrigin, synchMode);
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


Macro::Macro(QObject *parent)
    : QStateMachine(parent), stopped(false)
{
//    connect(this,SIGNAL(started()),sessionManager,SLOT(macroStarted()));
//    connect(this,SIGNAL(finished()),sessionManager,SLOT(macroEnded()));
//    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}



MacroState::MacroState(Macro *macro, JobOrigin jobOrigin)
    : QState(macro), macro(macro), jobOrigin(jobOrigin)
{
    connect(this,SIGNAL(entered()),this,SLOT(emitEnteredWithJobOrigin()));
}

void MacroState::emitEnteredWithJobOrigin()
{
    emit enteredWithJobOrigin(jobOrigin);
}



UserState::UserState(Macro *macro, JobOrigin jobOrigin)
    : MacroState(macro, jobOrigin)
{
    connect(this,SIGNAL(entered()),this,SLOT(deleteIfStopped()));
}



void UserState::deleteIfStopped()
{
    if (jobOrigin == JobOrigin::User && macro->isStopped())
        macro->deleteLater();
}


GUIState::GUIState(Macro *macro, JobOrigin jobOrigin)
    : MacroState(macro, jobOrigin)
{
}


QueryState::QueryState(Macro *macro, JobOrigin jobOrigin)
    : GUIState(macro, jobOrigin)
{
    connect(this,SIGNAL(exited()),this,SIGNAL(triggerProcessLazyNutOutput()));
}



void MacroQueue::run(Macro *macro)
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









