#ifndef CMDOUTPUT_H
#define CMDOUTPUT_H

#include <QPlainTextEdit>

class CmdOutput : public QPlainTextEdit
{
    Q_OBJECT

public:
    CmdOutput(QWidget *parent = 0);

public slots:
    void displayOutput(const QString & output);

};

#endif // CMDOUTPUT_H
