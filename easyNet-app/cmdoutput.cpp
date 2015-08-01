#include "cmdoutput.h"

CmdOutput::CmdOutput(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setStyleSheet("background-color : black; color : white;");
    QFont qf("Courier");
    setFont(qf);
}

void CmdOutput::displayOutput(const QString & output)
{
    // http://stackoverflow.com/questions/13559990/qt-append-text-to-qplaintextedit-without-adding-newline-and-keep-scroll-at-the

    moveCursor (QTextCursor::End);
    insertPlainText (output);
    moveCursor (QTextCursor::End);
}

