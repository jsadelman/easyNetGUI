#include "sessionmanager.h"
#include "commandsequencer.h"
#include "jobqueue.h"
#include "treeitem.h"
#include "treemodel.h"
#include "lazynutobj.h"
#include "querycontext.h"
#include "driver.h"
//#include "lazynutoutputprocessor.h"
#include "lazynut.h"


#include <QtGlobal>
#include <QFinalState>
#include <QFileInfo>
#include <QDebug>

class MacroQueue;
typedef QHash<QString,LazyNutObj*> LazyNutObjCatalogue;



SessionManager::SessionManager(LazyNutObjCatalogue* objCatalogue, TreeModel* objTaxonomyModel, QObject *parent)
    : objCatalogue(objCatalogue), objTaxonomyModel(objTaxonomyModel), QObject(parent),
      lazyNutHeaderBuffer(""), lazyNutOutput(""), OOBrex("OOB secret: (\\w+)\\n")
{
    initParser();
//    context = new QueryContext;
//    driver = new lazyNutOutputParser::Driver(*context);
//    lazyNutOutputProcessor = new LazyNutOutputProcessor(objCatalogue,objTaxonomyModel,this);
//    commandSequencer = new CommandSequencer(lazyNut, this);
    macroQueue = new MacroQueue;

//    connect(commandSequencer,SIGNAL(commandsExecuted(QString,JobOrigin)),
//            this,SLOT(dispatchLazyNutOutput(QString,JobOrigin)));
//    connect(lazyNutOutputProcessor, SIGNAL(lazyNutOutputProcessed()),
//            this,SIGNAL(commandsExecuted()));
//    connect(lazyNutOutputProcessor,SIGNAL(beginObjHashModified()),
//            this,SIGNAL(beginObjHashModified()));
//    connect(lazyNutOutputProcessor,SIGNAL(endObjHashModified()),
//            this,SIGNAL(endObjHashModified()));




//    connect(commandSequencer,SIGNAL(currentReceivedCount(int)),
//            this,SIGNAL(currentReceivedCount(int)));
//    connect(commandSequencer,SIGNAL(isReady(bool)),
//            this,SIGNAL(isReady(bool)));
//    connect(this,SIGNAL(commandExecuted(QString)),
    //            commandSequencer,SLOT(receiveResult(QString)));
}

void SessionManager::initParser()
{
    context = new QueryContext;
    driver = new lazyNutOutputParser::Driver(*context);
}

void SessionManager::startLazyNut(QString lazyNutBat)
{
    lazyNut = new LazyNut(this);
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
    emit userLazyNutOutputReady(lazyNutOutput);
    lazyNutHeaderBuffer.append(lazyNutOutput);
    if (lazyNutHeaderBuffer.contains(OOBrex))
    {
        OOBsecret = OOBrex.cap(1);
        lazyNutHeaderBuffer.clear();
        disconnect(lazyNut,SIGNAL(outputReady(QString)),this,SLOT(getOOB(QString)));
        startCommandSequencer();
    }
}

void SessionManager::startCommandSequencer()
{
    commandSequencer = new CommandSequencer(lazyNut, this);
    connect(commandSequencer,SIGNAL(commandsExecuted(QString,JobOrigin)),
            this,SLOT(dispatchLazyNutOutput(QString,JobOrigin)));
    connect(commandSequencer,SIGNAL(userLazyNutOutputReady(QString)),
            this,SIGNAL(userLazyNutOutputReady(QString)));
}


void SessionManager::processLazyNutOutput(QString lno)
{
    lazyNutOutput = lno;
    bool parsingSuccessful = parseLazyNutOutput();
    emit lazyNutOutputParsed(parsingSuccessful);
    if (parsingSuccessful)
    {
        updateObjects();
        emit commandsExecuted();
    }
}



bool SessionManager::parseLazyNutOutput()
{
    return driver->parse_string(lazyNutOutput.toStdString(), "lazyNutOutput");
}


void SessionManager::updateObjects()
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




//void SessionManager::parseLazyNutOutput(const QString &lazyNutOutput)
//{
//    lazyNutBuffer.append(lazyNutOutput);
//    int indexInLazyNutBuffer = rxEND.indexIn(lazyNutBuffer);
//    int lengthRemainder;
//    while (indexInLazyNutBuffer >=0)
//    {
//        lengthRemainder = lazyNutBuffer.size() - indexInLazyNutBuffer - rxEND.matchedLength();
//        QString remainder = lazyNutBuffer.right(lengthRemainder);
//        lazyNutBuffer.chop(lengthRemainder);
//        bool parseSuccessful = driver->parse_string(lazyNutBuffer.toStdString(), "lazyNutOutput");
//        if (parseSuccessful)
//            emit commandExecuted(lazyNutBuffer);
//        else
//            qDebug() << lazyNutBuffer;
//        lazyNutBuffer = remainder;
//        indexInLazyNutBuffer = rxEND.indexIn(lazyNutBuffer);
//    }
//}

//void SessionManager::processLazyNutOutput()
//{
//    bool objHashModified = false;
//    foreach (TreeItem* queryItem, context->root->children())
//    {
//        QString queryType = queryItem->data(0).toString();
//        if (queryType == "subtypes")
//        {
//            QString objectType = queryItem->data(1).toString();
//            QModelIndex objectIndex = objTaxonomyModel->index(0,0);
//            int row = 0;
//            while (objTaxonomyModel->data(objTaxonomyModel->index(row,0,objectIndex),Qt::DisplayRole).toString() != objectType &&
//                   row < objTaxonomyModel->rowCount(objectIndex))
//                ++row;
//            QModelIndex typeIndex = objTaxonomyModel->index(row,0,objectIndex);
//            if (typeIndex.isValid())
//            {
//                foreach (TreeItem* subtypeItem, queryItem->children())
//                {
//                    objTaxonomyModel->appendValue(subtypeItem->data(1).toString(),typeIndex);
//                }
//            }
//        }
//        else if (queryType == "recently_modified")
//        {
//            foreach (TreeItem* objectItem, queryItem->children())
//            {
//                recentlyModified.append(objectItem->data(1).toString());
//            }
//        }
//        else if (queryType == "description")
//        {
//            emit beginObjHashModified();
//            foreach (TreeItem* objectItem, queryItem->children())
//            {
//                QString objectName = objectItem->data(1).toString();
//                objCatalogue->insert(objectName,new LazyNutObj());
//                foreach (TreeItem* propertyItem, objectItem->children())
//                {
//                    QString propertyKey = propertyItem->data(0).toString();
//                    QString propertyValue = propertyItem->data(1).toString();
//                    if (propertyValue.startsWith('[') && propertyValue.endsWith(']'))
//                        // todo: generate query list
//                        (*objCatalogue)[objectName]->appendProperty(propertyKey,propertyValue);
//                    else if (propertyValue.contains(','))
//                        (*objCatalogue)[objectName]->appendProperty(propertyKey,propertyValue.split(", "));
//                    else
//                        (*objCatalogue)[objectName]->appendProperty(propertyKey,propertyValue);
//                }
//            }
//            objHashModified = true;
//        }
//    }
//    if (objHashModified)
//        emit endObjHashModified();
//    context->clearQueries();
//}

void SessionManager::dispatchLazyNutOutput(QString lazyNutOutput, JobOrigin jobOrigin)
{
    if (jobOrigin == JobOrigin::User)
        emit commandsExecuted();
    else
        processLazyNutOutput(lazyNutOutput);
}

void SessionManager::killLazyNut()
{
    lazyNut->kill();
}





void SessionManager::runCommands()
{
    commandSequencer->runCommands(commandList, JobOrigin::User);
}

QStateMachine *SessionManager::buildMacro()
{
    QStateMachine * macro = new QStateMachine(this);
    connect(macro,SIGNAL(started()),this,SLOT(macroStarted()));
    connect(macro,SIGNAL(finished()),this,SLOT(macroEnded()));
    connect(macro,SIGNAL(finished()),macro,SLOT(deleteLater()));
    return macro;
}

//QueryState *SessionManager::buildQueryState(Macro *macro)
//{
//    QueryState * queryState = new QueryState(macro);
//    connect(queryState,SIGNAL(exited()),this,SLOT(processLazyNutOutput()));
//    return queryState;
//}


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
    QFinalState *finalState = new QFinalState(macro);
    macro->setInitialState(runCommandsState);

    // transitions
    runCommandsState->addTransition(this,SIGNAL(commandsExecuted()),getSubtypesState);
    getSubtypesState->addTransition(this,SIGNAL(commandsExecuted()),getRecentlyModifiedState);
    getRecentlyModifiedState->addTransition(this,SIGNAL(commandsExecuted()),getDescriptionsState);
    getDescriptionsState->addTransition(this,SIGNAL(commandsExecuted()),finalState);
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
    QFinalState *finalState = new QFinalState(macro);
    macro->setInitialState(runCommandsState);

    // transitions
    runCommandsState->addTransition(this,SIGNAL(commandsExecuted()),getRecentlyModifiedState);
    getRecentlyModifiedState->addTransition(this,SIGNAL(commandsExecuted()),getDescriptionsState);
    getDescriptionsState->addTransition(this,SIGNAL(commandsExecuted()),finalState);
    getDescriptionsState->addTransition(this,SIGNAL(skipDescriptions()),finalState);

    macroQueue->tryRun(macro);
}



bool SessionManager::getStatus()
{
    return commandSequencer->getStatus();
}

//int SessionManager::getCurrentReceivedCount()
//{
//    return commandSequencer->getCurrentReceivedCount();
//}

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
        commandList.append(QString("query 1 subtypes %1").arg(type));
    commandSequencer->runCommands(commandList, JobOrigin::GUI);
}

void SessionManager::getRecentlyModified()
{
    commandList.clear();
    commandList << "query 1 recently_modified"
                << "query 1 clear_recently_modified";
    commandSequencer->runCommands(commandList, JobOrigin::GUI);
}

void SessionManager::clearRecentlyModified()
{
    commandList.clear();
    commandList << "query 1 clear_recently_modified";
    commandSequencer->runCommands(commandList, JobOrigin::GUI);
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
        commandSequencer->runCommands(commandList, JobOrigin::GUI);
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


//Macro::Macro(QObject *parent)
//    : QStateMachine(parent), stopped(false)
//{
// //    connect(this,SIGNAL(started()),sessionManager,SLOT(macroStarted()));
// //    connect(this,SIGNAL(finished()),sessionManager,SLOT(macroEnded()));
// //    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
//}



//MacroState::MacroState(Macro *macro, JobOrigin jobOrigin)
//    : QState(macro), macro(macro), jobOrigin(jobOrigin)
//{
//    connect(this,SIGNAL(entered()),this,SLOT(emitEnteredWithJobOrigin()));
//}

//void MacroState::emitEnteredWithJobOrigin()
//{
//    emit enteredWithJobOrigin(jobOrigin);
//}



//UserState::UserState(Macro *macro)
//    : MacroState(macro, JobOrigin::User)
//{
//    connect(this,SIGNAL(entered()),this,SLOT(deleteMacroIfStopped()));
//}



//void UserState::deleteMacroIfStopped()
//{
//    if (jobOrigin == JobOrigin::User && macro->isStopped())
//        macro->deleteLater();
//}


//GUIState::GUIState(Macro *macro)
//    : MacroState(macro, JobOrigin::GUI)
//{
//}


//QueryState::QueryState(Macro *macro, JobOrigin jobOrigin)
//    : GUIState(macro, jobOrigin)
//{
//}



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









