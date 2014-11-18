
#include <QtWidgets>
#include <QtGlobal>
#include <QtDebug>
#include <QTreeView>
#include <QStringList>
#include <QEventLoop>

#include <iostream>
#include <fstream>


#include "driver.h"
//#include "parsenode.h"
#include "treeitem.h"
#include "querycontext.h"
#include "nmConsole.h"
#include "treemodel.h"
#include "lazynutobj.h"
#include "objexplorer.h"
#include "designwindow.h"
#include "lazynut.h"
#include "commandsequencer.h"

InputCmdLine::InputCmdLine(QWidget *parent)
    : QLineEdit(parent)
{
    connect(this,SIGNAL(returnPressed()),
            this,SLOT(sendCommand()));
}

void InputCmdLine::sendCommand()
{
    QString line = text();
    emit commandReady(line);
    clear();
}

CmdOutput::CmdOutput(QWidget *parent)
    : QPlainTextEdit(parent)
{
}

void CmdOutput::displayOutput(const QString & output)
{
    // http://stackoverflow.com/questions/13559990/qt-append-text-to-qplaintextedit-without-adding-newline-and-keep-scroll-at-the

    moveCursor (QTextCursor::End);
    insertPlainText (output);
    moveCursor (QTextCursor::End);
}





NM::NM(QObject *parent)
    : QProcess(parent)
{
}

NM::~NM()
{
    terminate();
}

void NM::sendCommand(const QString & line)
{
    write(qPrintable(line + "\n"));
}


void NM::getNMError()
{
    QByteArray bytes = readAllStandardError();
    emit outputReady(QString(bytes));
}



QueryProcessor::QueryProcessor(LazyNutObjCatalogue* objHash, TreeModel* objTaxonomyModel,  QWidget *parent):
    QSplitter(parent), objHash(objHash), objTaxonomyModel(objTaxonomyModel),
    contextQuerySize(0), lazyNutBuffer(""), treeOutput(""),
    //rxEND(".*END:[^\\n]*\\n$")
    rxEND("END:[^\\n]*\\n")
{
    context = new QueryContext;
    driver = new lazyNutOutput::Driver(*context);

    parseOutput = new CmdOutput(this);
    parseOutput->setReadOnly(true);
    addWidget(parseOutput);

    connect(this,SIGNAL(treeReady(QString)),parseOutput,SLOT(displayOutput(QString)));
}

void QueryProcessor::testDesignWindow()
{
    emit beginObjHashModified();
    objHash->insert("layerA",new LazyNutObj());
    (*objHash)["layerA"]->appendProperty(QString{"name"},QVariant{"layerA"});
    (*objHash)["layerA"]->appendProperty("type","layer");
    (*objHash)["layerA"]->appendProperty("subtype","iac_layer");
    (*objHash)["layerA"]->appendProperty("length", 24);
    (*objHash)["layerA"]->appendProperty("incoming connections","connectionBA");

    objHash->insert("layerB",new LazyNutObj());
    (*objHash)["layerB"]->appendProperty("name","layerB");
    (*objHash)["layerB"]->appendProperty("type","layer");
    (*objHash)["layerB"]->appendProperty("subtype","iac_layer");
    (*objHash)["layerB"]->appendProperty("length", 24);
    (*objHash)["layerB"]->appendProperty("incoming connections",QStringList{"connectionAB","biasAB"});

    objHash->insert("connectionAB",new LazyNutObj());
    (*objHash)["connectionAB"]->appendProperty("name","connectionAB");
    (*objHash)["connectionAB"]->appendProperty("type","connection");
    (*objHash)["connectionAB"]->appendProperty("subtype","connection");
    (*objHash)["connectionAB"]->appendProperty("length", 24);
    (*objHash)["connectionAB"]->appendProperty("Source","layerA");
    (*objHash)["connectionAB"]->appendProperty("Target","layerB");

    emit endObjHashModified();

    emit beginObjHashModified();
    objHash->remove("connectionAB");
    emit endObjHashModified();

}


void QueryProcessor::getTree(const QString &lazyNutOutput)
{
    lazyNutBuffer.append(lazyNutOutput);
    int indexInLazyNutBuffer = rxEND.indexIn(lazyNutBuffer);
    int lengthRemainder;
    while (indexInLazyNutBuffer >=0)
    {
        lengthRemainder = lazyNutBuffer.size() - indexInLazyNutBuffer - rxEND.matchedLength();
        QString remainder = lazyNutBuffer.right(lengthRemainder);
        lazyNutBuffer.chop(lengthRemainder);
        //qDebug() << lazyNutBuffer;
        //qDebug() << lengthRemainder;
        //qDebug() << remainder;
        bool result = driver->parse_string(lazyNutBuffer.toStdString(), "lazyNutOutput");
        if (result)
        {
            contextQuerySize = 0;
            treeOutput = "";
            if (context->root->childCount() > contextQuerySize)
            {
                for (int qi = contextQuerySize; qi < context->root->childCount(); ++qi)
                {
                    context->root->child(qi)->print(treeOutput);
                }
                //contextQuerySize = context->root->childCount();
                //lazyNutBuffer = remainder; //"";
           }
           else
           {
               // treeOutput += ".";
           }
           //qDebug() << treeOutput;
           emit treeReady(treeOutput);
           processQueries();
           emit resultAvailable(lazyNutBuffer);

        }
        else
        {
            //lazyNutBuffer.append(remainder);
            //lazyNutBuffer = remainder;
            qDebug() << "no result";
        /*    if (context->begin_query)
            {
                context->root->removeLastChild();
            }
            context->begin_query = false;
            lazyNutBuffer = ""; // to be deleted */
        }
        lazyNutBuffer = remainder;
        indexInLazyNutBuffer = rxEND.indexIn(lazyNutBuffer);
    }
}

void QueryProcessor::processQueries()
{
    QStringList recentlyModified;
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
                objHash->insert(objectName,new LazyNutObj());
                foreach (TreeItem* propertyItem, objectItem->children())
                {
                    QString propertyKey = propertyItem->data(0).toString();
                    QString propertyValue = propertyItem->data(1).toString();
                    if (propertyValue.startsWith('[') && propertyValue.endsWith(']'))
                        // todo: generate query list
                        (*objHash)[objectName]->appendProperty(propertyKey,propertyValue);
                    else if (propertyValue.contains(','))
                        (*objHash)[objectName]->appendProperty(propertyKey,propertyValue.split(", "));
                    else
                        (*objHash)[objectName]->appendProperty(propertyKey,propertyValue);
                }
            }
            objHashModified = true;
        }
    }
    if (objHashModified)
        emit endObjHashModified();
    context->clearQueries();
    foreach (QString objectName, recentlyModified)
    {
        QString query = "query 1 " + objectName + " description";
        emit commandReady(query);
    }
    recentlyModified.clear();
}


NmCmd::NmCmd(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    cmdOutput = new CmdOutput(this);
    cmdOutput->setReadOnly(true);
    inputCmdLine = new InputCmdLine(this);
    mainLayout->addWidget(cmdOutput);
    mainLayout->addWidget(inputCmdLine);
    setLayout(mainLayout);
    setWindowTitle(tr("lazyNut cmd[*]"));
}


NmConsole::NmConsole(QWidget *parent)
    : QMainWindow(parent)
{
    nmCmd = new NmCmd(this);
    setCentralWidget(nmCmd);
    scriptEdit = new QPlainTextEdit(this);
    scriptEdit->setReadOnly(true);
    dockEdit = new QDockWidget(tr("my_nm_script"), this);
    dockEdit->setAllowedAreas(  Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea);
    dockEdit->setWidget(scriptEdit);
    addDockWidget(Qt::RightDockWidgetArea, dockEdit);
    dockEdit->hide();

    objTaxonomyModel = new TreeModel(QStringList{"Object taxonomy"},this);
    objTaxonomyModel->appendValue(QString{"object"});
    // type
    QModelIndex parentIndex = objTaxonomyModel->index(0,0);
    foreach (QString type, lazyNutObjTypes)
        objTaxonomyModel->appendValue(type,parentIndex);

    objHash = new LazyNutObjCatalogue();

    queryProcessor = new QueryProcessor(objHash,objTaxonomyModel,this);
    objExplorer = new ObjExplorer(objHash,objTaxonomyModel,this);

    dockExplorer = new QDockWidget(tr("Object Explorer"), this);
    dockExplorer->setAllowedAreas( Qt::BottomDockWidgetArea );
    dockExplorer->setWidget(objExplorer);
    addDockWidget(Qt::BottomDockWidgetArea,dockExplorer);



    designWindow = new DesignWindow(this);
    designWindow->setObjCatalogue(objHash);
    connect(queryProcessor,SIGNAL(endObjHashModified()),
            designWindow,SLOT(objCatalogueChanged()));
    connect(designWindow,SIGNAL(showObj(LazyNutObj*,LazyNutObjCatalogue*)),
            objExplorer,SLOT(setObj(LazyNutObj*,LazyNutObjCatalogue*)));
    connect(objExplorer,SIGNAL(objSelected(QString)),
            designWindow,SIGNAL(objSelected(QString)));

    dockDesignWindow = new QDockWidget(tr("Design Window"), this);
    dockDesignWindow->setAllowedAreas(  Qt::LeftDockWidgetArea |
                                    Qt::RightDockWidgetArea);
    dockDesignWindow->setWidget(designWindow);
    addDockWidget(Qt::LeftDockWidgetArea, dockDesignWindow);
        //dockDesignWindow->hide();


    // signals begin/endObjHashModified are defined in QueryProcessor and ObjExplorer.
    // When a description query is parsed by the QueryProcessor, it sends those signals
    // to the ObjExplorer, which in turn trigger slots sendBegin/EndResetModel in
    // LazyNutObjTableModel that update its internal representation.
    // All this signal/slot chain is necessary in order to call
    // the protected functions QAbstractItemModel::begin/endResetModel.
    // Those functions need to be called because QueryProcessor modifies
    // the LazyNutObjCatalogue object directly, which is the underlying data structure
    // to LazyNutObjTableModel, i.e. it does not make use of the QAbstractTableModel
    // API implemented by LazyNutObjTableModel.

    connect(queryProcessor,SIGNAL(beginObjHashModified()),objExplorer,SIGNAL(beginObjHashModified()));
    connect(queryProcessor,SIGNAL(endObjHashModified()),objExplorer,SIGNAL(endObjHashModified()));



    createActions();
    createMenus();
    createToolBars();

    readSettings();

    //queryProcessor->testDesignWindow();

    setCurrentFile("");




    //lazyNut = new NM(this);
    lazyNut = new LazyNut(this);



    connect(lazyNut,SIGNAL(outputReady(QString)),
            nmCmd->cmdOutput,SLOT(displayOutput(QString)));
    connect(lazyNut,SIGNAL(outputReady(QString)),
            queryProcessor,SLOT(getTree(QString)));

    connect(this,SIGNAL(savedLayoutToBeLoaded(QString)),
            designWindow,SIGNAL(savedLayoutToBeLoaded(QString)));
    connect(this,SIGNAL(saveLayout()),
            designWindow,SIGNAL(saveLayout()));

    //easyNetHome = QString(qgetenv("EASYNET_HOME"));
    //lazyNutBat = ""; // debug
    //easyNetHome = ""; // debug
    if (lazyNutBat.isEmpty())
    {
        if (easyNetHome.isEmpty())
        {
            QMessageBox::warning(this, "warning",QString("Please select a valid %1 file from the menu Settings -> Set %1\n"
                                 "or a valid easyNet home directory using the menu Settings -> Set easyNet home directory").arg(lazyNutBasename));
        }
        else
        {
            lazyNutBat = easyNetHome + QString("/%1/nm_files/%2").arg(binDir).arg(lazyNutBasename);
        }
    }
    if (!lazyNutBat.isEmpty())
        runLazyNutBat();

    commandSequencer = new CommandSequencer(lazyNut,this);
    connect(queryProcessor,SIGNAL(resultAvailable(QString)),
            commandSequencer,SLOT(receiveResult(QString)));
    connect(nmCmd->inputCmdLine,SIGNAL(commandReady(QString)),
            commandSequencer,SLOT(runCommand(QString)));
    connect(queryProcessor,SIGNAL(commandReady(QString)),
            commandSequencer,SLOT(runCommand(QString)));

}

void NmConsole::readSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    easyNetHome = settings.value("easyNetHome","").toString();
    lazyNutBat = settings.value("lazyNutBat","").toString();
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    scriptsDir = settings.value("scriptsDir", easyNetHome + "/Models").toString();
    resize(size);
    move(pos);
}

void NmConsole::writeSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    settings.setValue("easyNetHome", easyNetHome);
    settings.setValue("lazyNutBat",lazyNutBat);
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("scriptsDir",scriptsDir);
    //  settings.setValue("nmExe",nmExe);
}



void NmConsole::closeEvent(QCloseEvent *event)
{
        writeSettings();
        lazyNut->kill();
        // wait until the scene has saved its layout
//        QEventLoop waitUntilLayoutSaved;
//        connect(designWindow,SIGNAL(layoutSaveAttempted()),
//                &waitUntilLayoutSaved,SLOT(quit()));
        emit saveLayout();
//        waitUntilLayoutSaved.exec();
        event->accept();
}

/*void NmConsole::newFile()
{
    if (maybeSave()) {
        scriptEdit->clear();
        setCurrentFile("");
    }
}*/

void NmConsole::open()
{
 //   if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,tr("Open script"), scriptsDir, tr("Script Files (*.eNs *.eNm)"));
        if (!fileName.isEmpty())
        {
            loadFile(fileName);
            dockEdit->show();
        }
 //   }
}

/*bool NmConsole::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool NmConsole::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.exec();
    QStringList files = dialog.selectedFiles();

    if (files.isEmpty())
        return false;

    return saveFile(files.at(0));
}*/

void NmConsole::runSelection()
{
    chopAndSend(scriptEdit->textCursor().selectedText());
}

void NmConsole::runScript()
{
    chopAndSend(scriptEdit->toPlainText());
    QStringList commandList;
    foreach (QString type, lazyNutObjTypes)
        commandList.append("query 1 subtypes " + type);
    commandList.append("query 1 recently_modified");
    commandSequencer->runCommands(commandList);
    emit savedLayoutToBeLoaded(curJson);
}

void NmConsole::setEasyNetHome()
{
    easyNetHome = QFileDialog::getExistingDirectory(this,tr("Please select your easyNet home directory.\n"));
    lazyNutBat = easyNetHome + QString("/%1/nm_files/%2").arg(binDir).arg(lazyNutBasename);
    runLazyNutBat();
}

void NmConsole::setLazyNutBat()
{
    lazyNutBat = QFileDialog::getOpenFileName(this,QString(tr("Please select your %1 file.")).arg(lazyNutBasename),
                                              easyNetHome,QString("*.%1").arg(lazyNutExt));
    runLazyNutBat();
}


void NmConsole::runLazyNutBat()
{
    lazyNut->setWorkingDirectory(QFileInfo(lazyNutBat).absolutePath());
    lazyNut->start(lazyNutBat);
    if (lazyNut->state() == QProcess::NotRunning)
    {
        QMessageBox::critical(this, "critical",
        QString("%1 script not running or not found.\n"
                "Please select a valid %1 file from the menu Settings -> Set %1\n"
                "or a valid easyNet home directory using the menu Settings -> Set easyNet home directory").arg(lazyNutBasename));
    }
}

void NmConsole::chopAndSend(const QString & text)
{
    QStringList commandList = text.split("\u2029");
    commandSequencer->runCommands(commandList);
//    QStringList lines = text.split("\u2029");
//    foreach (QString line, lines)
//    {
//        lazyNut->sendCommand(line + "\n");
//    }
}


/*void NmConsole::documentWasModified()
{
    setWindowModified(scriptEdit->document()->isModified());
}*/


void NmConsole::createActions()
{
    //newAct = new QAction(tr("&New"), this);
    //newAct->setShortcuts(QKeySequence::New);
    //newAct->setStatusTip(tr("Create a new file"));
    //connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    //saveAct = new QAction(tr("&Save"), this);
    //saveAct->setShortcuts(QKeySequence::Save);
    //saveAct->setStatusTip(tr("Save the document to disk"));
    //connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    //saveAsAct = new QAction(tr("Save &As..."), this);
    //saveAsAct->setShortcuts(QKeySequence::SaveAs);
    //saveAsAct->setStatusTip(tr("Save the document under a new name"));
    //connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    runSelectionAct = new QAction(tr("Run se&lection"), this);
    runSelectionAct->setStatusTip(tr("Run selected text"));
    connect(runSelectionAct,SIGNAL(triggered()),this, SLOT(runSelection()));

    runScriptAct = new QAction(tr("&Run script"), this);
    runScriptAct->setStatusTip(tr("Run script"));
    connect(runScriptAct,SIGNAL(triggered()),this, SLOT(runScript()));

    setEasyNetHomeAct = new QAction(tr("Set easyNet home directory"), this);
    setEasyNetHomeAct->setStatusTip(tr("Set easyNet home directory"));
    connect(setEasyNetHomeAct,SIGNAL(triggered()),this, SLOT(setEasyNetHome()));

    setLazyNutBatAct = new QAction(QString(tr("Set %1").arg(lazyNutBasename)), this);
    setLazyNutBatAct->setStatusTip(QString(tr("Set %1").arg(lazyNutBasename)));
    connect(setLazyNutBatAct,SIGNAL(triggered()),this, SLOT(setLazyNutBat()));

}

void NmConsole::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    //fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    //fileMenu->addAction(saveAct);
    //fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    runMenu = menuBar()->addMenu(tr("&Run"));
    runMenu->addAction(runSelectionAct);
    runMenu->addAction(runScriptAct);

    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    settingsMenu->addAction(setEasyNetHomeAct);
    settingsMenu->addAction(setLazyNutBatAct);
}

void NmConsole::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(openAct);
    runToolBar = addToolBar(tr("Run"));
    runToolBar->addAction(runSelectionAct);
    runToolBar->addAction(runScriptAct);

}


void NmConsole::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    scriptEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    scriptsDir = QFileInfo(fileName).absolutePath();

    //statusBar()->showMessage(tr("File loaded"), 2000);
}

/*bool NmConsole::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("nm Console"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << scriptEdit->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    //statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}*/

void NmConsole::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    scriptEdit->document()->setModified(false);
    setWindowModified(false);

    QString shownName = curFile;
    //if (curFile.isEmpty())
    //    shownName = "untitled.txt";
    dockEdit->setWindowTitle(strippedName(shownName));
    curJson = QFileInfo(curFile).dir().filePath(QFileInfo(curFile).completeBaseName().append(".json"));
}

QString NmConsole::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

/*bool NmConsole::maybeSave()
{
    if (scriptEdit->document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("nm console"),
                     tr("The script has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}*/




