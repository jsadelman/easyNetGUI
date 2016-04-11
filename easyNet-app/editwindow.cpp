/****************************************************************************
**
**
****************************************************************************/

#include <QtWidgets>
#include <QPlainTextEdit>


#include "editwindow.h"
#include "codeeditor.h"
#include "finddialog.h"

EditWindow::EditWindow(QWidget *parent, bool isReadOnly)
    : QMainWindow(parent), isReadOnly(isReadOnly)

{
//    textEdit = new QPlainTextEdit;
    textEdit = new CodeEditor(this);
    textEdit->setReadOnly(isReadOnly);
    setCentralWidget(textEdit);
    textEdit->setReadOnly(isReadOnly);
    startDir="";

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

    setCurrentFile("Untitled");
    setUnifiedTitleAndToolBarOnMac(true);

    findDialog = new FindDialog;
    connect(findDialog, SIGNAL(findForward(QString, QFlags<QTextDocument::FindFlag>)),
            this, SLOT(findForward(QString, QFlags<QTextDocument::FindFlag>)));
    connect(findDialog, SIGNAL(findBackward(QString, QFlags<QTextDocument::FindFlag>)),
            this, SLOT(findBackward(QString, QFlags<QTextDocument::FindFlag>)));


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
    if (maybeSave())
    {
        // bring up file dialog
        QString fileName = QFileDialog::getOpenFileName(this,"",startDir);
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

    saveAsAct = new QAction(QIcon(":/images/save_as_download_disk.png"), tr("Save &As..."), this);
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

    copyAct = new QAction(QIcon(":/images/clipboard.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
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

    findAct = new QAction(QIcon(":/images/magnifying-glass-2x.png"), tr("&Find"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setToolTip(tr("Find text in this window"));
    connect(findAct, SIGNAL(triggered()), this, SLOT(showFindDialog()));
    findAct->setEnabled(true);



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
    filenameLabel = new QLabel("Untitled");
    fileToolBar->addWidget(filenameLabel);
    if (newAct)
        fileToolBar->addAction(newAct);
    if (openAct)
        fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(saveAsAct);


    editToolBar = addToolBar(tr("Edit"));
    if (!isReadOnly)
        editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    if (!isReadOnly)
        editToolBar->addAction(pasteAct);
    editToolBar->addAction(findAct);
}

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
    setFilenameLabel(strippedName(shownName));

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


void EditWindow::showFindDialog()
{
    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
}

void EditWindow::findForward(const QString &str, QFlags<QTextDocument::FindFlag> flags)
{
//    qDebug() << "findForward";
    bool result = textEdit->find(str, flags);
    if (!result)
    {
        textEdit->moveCursor(QTextCursor::Start);
        result = textEdit->find(str, flags);
        if (!result)
        {
            QMessageBox::warning(this, "Find",QString("The text was not found"));
//            findDialog->hide();
            findDialog->raise();
        }
    }
}

void EditWindow::findBackward(const QString &str, QFlags<QTextDocument::FindFlag> flags)
{
//    qDebug() << "findBackward";
    bool result = textEdit->find(str,flags);
    if (!result)
    {
        textEdit->moveCursor(QTextCursor::End);
        result = textEdit->find(str,flags);
        if (!result)
        {
            QMessageBox::warning(this, "Find",QString("The text was not found"));
            findDialog->hide();
        }
    }
}

void EditWindow::setFilenameLabel(QString filename)
{
    QString name = QString("<P><b>") + filename + QString("<\b>");
    filenameLabel->setText(name);
}
