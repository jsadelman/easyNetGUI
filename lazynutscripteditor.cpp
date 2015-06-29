#include "lazynutscripteditor.h"

#include <QTextStream>

LazyNutScriptEditor::LazyNutScriptEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(true);
}

LazyNutScriptEditor::~LazyNutScriptEditor()
{

}

QStringList LazyNutScriptEditor::getSelectedText()
{
    return textCursor().selectedText().split("\u2029");
}

QStringList LazyNutScriptEditor::getAllText()
{
    return toPlainText().split("\n");
}

/*bool LazyNutScriptEditor::maybeSave()
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






