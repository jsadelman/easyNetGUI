/****************************************************************************
**
**
****************************************************************************/

#include <QtWidgets>

#include "editwindow.h"
#include "codeeditor.h"

EditWindow::EditWindow(QWidget *parent, QAction *p_newAct, QAction *p_openAct, bool isReadOnly)
    : QMainWindow(parent), isReadOnly(isReadOnly)

{
//    textEdit = new QPlainTextEdit;
    textEdit = new CodeEditor(this);
    setCentralWidget(textEdit);
    textEdit->setReadOnly(isReadOnly);
    setCurrentFile("Untitled");

//    newAct = p_newAct;
//    openAct = p_openAct;

    //cutAllowed = a_cutAllowed;
    //pasteAllowed = a_pasteAllowed;
    createActions();
//    createMenus();
    createToolBars();
//    createStatusBar();

//    readSettings();

    connect(textEdit->document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));

    setCurrentFile("");
    setUnifiedTitleAndToolBarOnMac(true);
}

void EditWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
//        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void EditWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFile("");
    }
}

void EditWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool EditWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool EditWindow::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList files;
    if (dialog.exec())
        files = dialog.selectedFiles();
    else
        return false;

    return saveFile(files.at(0));
}

void EditWindow::documentWasModified()
{
    setWindowModified(textEdit->document()->isModified());
}

void EditWindow::createActions()
{
    newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
//    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));


      openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
//    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));


    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
//    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
//    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    if (!isReadOnly)
    {
    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
//    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
//                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));
    cutAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));

    }

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
//    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
//                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));

    if (!isReadOnly)
    {
    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
//    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
//                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));
    }

/*    findAct = new QAction(QIcon(":/images/find.png"), tr("&Find"), this);
    copyAct->setShortcuts(QKeySequence::Find);
//    copyAct->setStatusTip(tr("Find text in this window"));
    connect(findAct, SIGNAL(triggered()), textEdit, SLOT(find()));
    findAct->setEnabled(false);
*/

    runAct = new QAction(QIcon(":/images/media-play-3x.png"), tr("&Run"), this);
//    runAct->setShortcuts(QKeySequence::New);
    runAct->setStatusTip(tr("Run script"));
    connect(runAct, SIGNAL(triggered()), this, SLOT(runScript()));

    runSelectionAct = new QAction(QIcon(":/images/reload-2x.png"),tr("Run se&lection"), this);
    runSelectionAct->setStatusTip(tr("Run selected text"));
    connect(runSelectionAct,SIGNAL(triggered()),this, SLOT(runSelection()));

    stopAct = new QAction("STOP",this);
//    connect(stopAct,SIGNAL(triggered()),SessionManager::instance(),SLOT(stop()));
    pauseAct = new QAction("PAUSE",this);
//    connect(pauseAct,SIGNAL(triggered()),SessionManager::instance(),SLOT(pause()));
//    connect(SessionManager::instance(),SIGNAL(isPaused(bool)),this,SLOT(showPauseState(bool)));

}

/*
void editWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    if (cutAllowed)
        editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    if (pasteAllowed)
        editMenu->addAction(pasteAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}
*/

void EditWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    if (newAct)
        fileToolBar->addAction(newAct);
    if (openAct)
        fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    if (!isReadOnly)
        editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    if (!isReadOnly)
        editToolBar->addAction(pasteAct);

    runToolBar = addToolBar(tr("Run"));
    runToolBar->addAction(runAct);
    runToolBar->addAction(runSelectionAct);
//    runToolBar->addAction(pasteAct);

}

/*
void editWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void editWindow::readSettings()
{
    QSettings settings("QtProject", "Application Example");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
}

void editWindow::writeSettings()
{
    QSettings settings("QtProject", "Application Example");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}
*/

bool EditWindow::maybeSave()
{
    if (textEdit->document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"),
                     tr("The document has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void EditWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
//    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool EditWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << textEdit->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
//    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void EditWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    textEdit->document()->setModified(false);
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
    setWindowTitle(strippedName(shownName));

}


QString EditWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}


void EditWindow::addText(QString txt)
{
    textEdit->moveCursor(QTextCursor::End);
    textEdit->appendPlainText(txt);
    textEdit->moveCursor(QTextCursor::End);
}

void EditWindow::runScript()
{
    emit runCmdAndUpdate(textEdit->getAllText());
}

void EditWindow::runSelection()
{
    emit runCmdAndUpdate(textEdit->getSelectedText());
}
