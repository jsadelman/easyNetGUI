
#include <QtWidgets>
#include <QtGlobal>
#include <QtDebug>
#include <QTreeView>
#include <QStringList>

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

InputLine::InputLine(QWidget *parent)
    : QLineEdit(parent)
{
}

void InputLine::sendLine()
{
    QString line = text();
    emit outputReady(line);
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

/*
ParseTree::ParseTree(QWidget *parent)
    : QTreeView(parent), treeModel(QStringList{"key","value"}, parent), //context(&treeModel), driver(context),
      contextQuerySize(0), lazyNutBuffer(""), treeOutput(""), rxEND(".*END:[^\\n]*\n$")
{
    //qDebug() << treeModel.getRootItem()->childCount();
    contextPtr = new QueryContext(&treeModel);
    driverPtr = new lazyNutOutput::Driver(*contextPtr);
    //treeView = new QTreeView(parent);
    setModel(&treeModel);
    //addWidget(treeView);
    parseOutput = new CmdOutput(this);
    parseOutput->setReadOnly(true);
    addWidget(parseOutput);

    connect(this,SIGNAL(treeReady(QString)),parseOutput,SLOT(displayOutput(QString)));

}

void ParseTree::getTree(const QString &lazyNutOutput)
{
    lazyNutBuffer.append(lazyNutOutput);
    if (rxEND.exactMatch(lazyNutBuffer)) {
        bool result = driverPtr->parse_string(lazyNutBuffer.toStdString(), "lazyNutOutput");
        if (result)
        {
            if (treeModel.rowCount() > contextQuerySize)
            {
                contextQuerySize = treeModel.rowCount();
                lazyNutBuffer = "";
            }
            treeOutput = "";
            if (contextPtr->root->childCount() > contextQuerySize)
            {
                for (int qi = contextQuerySize; qi < contextPtr->root->childCount(); ++qi)
                {
                    contextPtr->root->child(qi)->print(treeOutput);
                }
                contextQuerySize = contextPtr->root->childCount();
                lazyNutBuffer = "";
            }
            else
            {
                treeOutput += ".";
            }
            emit treeReady(treeOutput);
        }
        else
        {
            if (contextPtr->begin_query)
            {
                //context.root->removeLastChild();
                contextPtr->treeModelPtr->removeRows(contextPtr->treeModelPtr->rowCount()-1,1);
            }
            contextPtr->begin_query = false;
            lazyNutBuffer = ""; // to be deleted
        }
    }
}
*/

QueryProcessor::QueryProcessor(LazyNutObjCatalogue* objHash, TreeModel* objTaxonomyModel,  QWidget *parent):
    QSplitter(parent), objHash(objHash), objTaxonomyModel(objTaxonomyModel),
    contextQuerySize(0), lazyNutBuffer(""), treeOutput(""), rxEND(".*END:[^\\n]*\\n$")
{
    context = new QueryContext;
    driver = new lazyNutOutput::Driver(*context);

    parseOutput = new CmdOutput(this);
    parseOutput->setReadOnly(true);
    addWidget(parseOutput);

    connect(this,SIGNAL(treeReady(QString)),parseOutput,SLOT(displayOutput(QString)));
}

void QueryProcessor::getTree(const QString &lazyNutOutput)
{
    lazyNutBuffer.append(lazyNutOutput);
    if (rxEND.exactMatch(lazyNutBuffer))
    {
        //qDebug() << "################## START \n" << rxEND.cap() << "############# END\n";
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
                lazyNutBuffer = "";
           }
           else
           {
                treeOutput += ".";
           }
           emit treeReady(treeOutput);
           processQueries();
           lazyNutBuffer = "";
           //context->clearQueries();
        }
        else
        {
        /*    if (context->begin_query)
            {
                context->root->removeLastChild();
            }
            context->begin_query = false;
            lazyNutBuffer = ""; // to be deleted */
        }
    }
}

void QueryProcessor::processQueries()
{
    QStringList recentlyModified;
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
            emit endObjHashModified();
        }
    }
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
    inputLine = new InputLine(this);
    mainLayout->addWidget(cmdOutput);
    mainLayout->addWidget(inputLine);
    setLayout(mainLayout);
    setWindowTitle(tr("nm cmd[*]"));
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
    connect(queryProcessor,SIGNAL(endObjHashModified()),designWindow,SLOT(objCatalogueChanged()));
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



    //connect(scriptEdit->document(), SIGNAL(contentsChanged()),
    //            this, SLOT(documentWasModified()));

    setCurrentFile("");
    //scriptsDir = "C:/Users/mm14722/programs/easyNet_download/easynet-code-286-trunk/easynet-code-286-trunk/bin/GUI/Models";




    nm = new NM(this);
    connect(nmCmd->inputLine,SIGNAL(returnPressed()),
            nmCmd->inputLine,SLOT(sendLine()));
    connect(nmCmd->inputLine,SIGNAL(outputReady(QString)),
            nm,SLOT(sendCommand(QString)));
    connect(nm,SIGNAL(readyReadStandardError()),
            nm,SLOT(getNMError()));
    connect(nm,SIGNAL(outputReady(QString)),
            nmCmd->cmdOutput,SLOT(displayOutput(QString)));
    connect(nm,SIGNAL(outputReady(QString)),
            queryProcessor,SLOT(getTree(QString)));
    connect(queryProcessor,SIGNAL(commandReady(QString)),
            nm,SLOT(sendCommand(QString)));




    //QString nm_dir = "C:/Users/mm14722/programs/easyNet/bin/nm_files/";
    //QString nm_dir = "C:/Users/mm14722/programs/easyNet_download/easynet-code-286-trunk/easynet-code-286-trunk/bin/nm_files/";
    //QString nmExe = nm_dir + "nm.exe";
    QString easynet_home = QString(qgetenv("EASYNET_HOME"));
    if (easynet_home.isEmpty())
    {
        easynet_home = QFileDialog::getExistingDirectory(this,"Please select your easyNet home directory");
        if (!qputenv("EASYNET_HOME",easynet_home.toLocal8Bit()))
        {
            QMessageBox::warning(this, "warning", "easyNet home directory could not be set");
        }
    }

    nmExe = QString(qgetenv("EASYNET_HOME")).append("/bin/nm_files/lazyNut.bat");
    /*if (nmExe.isEmpty()) {
       nmExe = QFileDialog::getOpenFileName(this,tr("Select your nm exe"), "", tr("(*.exe *.bat)"));

    }*/
    nm->setWorkingDirectory(QFileInfo(nmExe).absolutePath());
    nm->start(nmExe);
    if (nm->state() != QProcess::Running)
    {
        QMessageBox::critical(this, "critical", "lazyNut.bat script not running or not found");
    }
}

void NmConsole::readSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    scriptsDir = settings.value("scriptsDir",qgetenv("EASYNET_HOME") + "/Models").toString();
    resize(size);
    move(pos);
}

void NmConsole::writeSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("scriptsDir",scriptsDir);
  //  settings.setValue("nmExe",nmExe);
}


void NmConsole::closeEvent(QCloseEvent *event)
{
//    if (maybeSave()) {
    //QSettings settings("QtEasyNet", "nmConsole");
    //settings.clear();
        writeSettings();
        nm->terminate();
        event->accept();
    //} else {
    //    event->ignore();
    //}
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
        QString fileName = QFileDialog::getOpenFileName(this,tr("Open script"), scriptsDir, tr("Script Files (*.eNs *eNm)"));
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
    foreach (QString type, lazyNutObjTypes)
        nm->sendCommand("query 1 subtypes " + type + "\n");
    nm->sendCommand("query 1 recently_modified\n");
}

void NmConsole::chopAndSend(const QString & text)
{
    QStringList lines = text.split("\u2029");
    foreach (QString line, lines)
    {
        nm->sendCommand(line + "\n");
    }
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

    //queryModeAct = new QAction(tr("&Query mode"), this);
    //connect(queryModeAct,SIGNAL(),this, SLOT(runScript()));

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
        QMessageBox::warning(this, tr("nm Console"),
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
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    dockEdit->setWindowTitle(strippedName(shownName));

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




