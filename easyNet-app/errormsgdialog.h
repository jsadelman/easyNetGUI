#ifndef ERRORMSGDIALOG_H
#define ERRORMSGDIALOG_H

#include <QDialog>

class QTextEdit;

class ErrorMsgDialog : public QDialog
{
    Q_OBJECT
public:
    ErrorMsgDialog(QWidget *parent = Q_NULLPTR);
    void setErrorLog(QString errorLog);

private:
    QTextEdit *errorLogDisplay;
    void build();
};

#endif // ERRORMSGDIALOG_H
