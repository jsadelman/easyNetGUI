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

void SessionManager::runCommands()
{
    commandSequencer->runCommands(commandList, synchMode);
}


void SessionManager::runModel(QStringList cmdList)
{
    commandList = cmdList;
    // states
    Macro * macro = new Macro(this,this);
    QState * runCommandsState = new QState(macro);
    connect(runCommandsState,SIGNAL(entered()),this,SLOT(runCommands()));
    QueryState * getSubtypesState = new QueryState(this,macro);
    connect(getSubtypesState,SIGNAL(entered()),this,SLOT(getSubtypes()));
    QueryState * getRecentlyModifiedState = new QueryState(this,macro);
    connect(getRecentlyModifiedState,SIGNAL(entered()),this,SLOT(getRecentlyModified()));

    QueryState * getDescriptionsState = new QueryState(this,macro);
    connect(getDescriptionsState,SIGNAL(entered()),this,SLOT(getDescriptions()));
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
    // states
    Macro * macro = new Macro(this,this);
    QState * runCommandsState = new QState(macro);
    connect(runCommandsState,SIGNAL(entered()),this,SLOT(runCommands()));
    QueryState * getRecentlyModifiedState = new QueryState(this,macro);
    connect(getRecentlyModifiedState,SIGNAL(entered()),this,SLOT(getRecentlyModified()));

    QueryState * getDescriptionsState = new QueryState(this,macro);
    connect(getDescriptionsState,SIGNAL(entered()),this,SLOT(getDescriptions()));
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
    commandSequencer->pause();
}

void SessionManager::stop()
{
    commandSequencer->stop();
    macroQueue->stop();
}

void SessionManager::getSubtypes()
{
    commandList.clear();
    foreach (QString type, lazyNutObjTypes)
        commandList.append(QString("query 1 subtypes %1").arg(type));
    commandSequencer->runCommands(commandList, synchMode);
}

void SessionManager::getRecentlyModified()
{
    commandList.clear();
    commandList << "query 1 recently_modified"
                << "query 1 clear_recently_modified";
    commandSequencer->runCommands(commandList, synchMode);
}

void SessionManager::clearRecentlyModified()
{
    commandList.clear();
    commandList << "query 1 clear_recently_modified";
    commandSequencer->runCommands(commandList, synchMode);
}

void SessionManager::getDescriptions()
{
    if (recentlyModified.isEmpty())
        emit skipDescriptions();
    else
    {
        commandList.clear();
        foreach (QString obj, recentlyModified)
            commandList.append(QString("query 1 %1 description").arg(obj));
        commandSequencer->runCommands(commandList);
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


Macro::Macro(SessionManager *sm, QObject *parent)
    : sessionManager(sm), QStateMachine(parent)
{
    connect(this,SIGNAL(started()),sessionManager,SLOT(macroStarted()));
    connect(this,SIGNAL(finished()),sessionManager,SLOT(macroEnded()));
    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}



QueryState::QueryState(SessionManager *sm, QState *parent)
    : sessionManager(sm), QState(parent)
{
    connect(this,SIGNAL(exited()),sessionManager,SLOT(processLazyNutOutput()));
}


//RunCommandsState::RunCommandsState(SessionManager *sm, QState *parent)
//    : sessionManager(sm), QState(parent)
//{
//    connect(this,SIGNAL(entered()),sessionManager,SLOT(runCommands()));
//}


//GetRecentlyModifiedState::GetRecentlyModifiedState(SessionManager *sm, QState *parent)
//    : sessionManager(sm), QState(parent)
//{
//    connect(this,SIGNAL(entered()),sessionManager,SLOT(getRecentlyModified()));
//    connect(this,SIGNAL(exited()),sessionManager,SLOT(setRecentlyModified()));
//}

//ClearRecentlyModifiedState::ClearRecentlyModifiedState(SessionManager *sm, QState *parent)
//    : sessionManager(sm), QState(parent)
//{
//    connect(this,SIGNAL(entered()),sessionManager,SLOT(clearRecentlyModified()));
//}


//GetRecentlyModifiedAndClearState::GetRecentlyModifiedAndClearState(SessionManager *sm, QState *parent)
//    : sessionManager(sm), QState(parent)
//{
//    GetRecentlyModifiedState *getRecentlyModifiedState = new GetRecentlyModifiedState(sm,this);
//    ClearRecentlyModifiedState *clearRecentlyModifiedState = new ClearRecentlyModifiedState(sm,this);
//    setInitialState(getRecentlyModifiedState);
//    getRecentlyModifiedState->addTransition(sessionManager->getCommandSequencer(),SIGNAL(commandsExecuted()),clearRecentlyModifiedState);


//}


//GetDescriptionsState::GetDescriptionsState(SessionManager *sm, QState *parent)
//    : sessionManager(sm), QState(parent)
//{
//    connect(this,SIGNAL(entered()),sessionManager,SLOT(getDescriptions()));
//    connect(this,SIGNAL(exited()),sessionManager,SLOT(processLazyNutOutput()));
//}



//GetSubtypesState::GetSubtypesState(SessionManager *sm, QState *parent)
//    : sessionManager(sm), QState(parent)
//{
//    connect(this,SIGNAL(entered()),sessionManager,SLOT(getSubtypes()));
//    connect(this,SIGNAL(exited()),sessionManager,SLOT(processLazyNutOutput()));
//}




void MacroQueue::run(Macro *macro)
{
    macro->start();
}

void MacroQueue::reset()
{
    while (!queue.isEmpty())
    {
        delete queue.dequeue();
    }
    delete currentJob;
}

QString MacroQueue::name()
{
    return "MacroQueue";
}






