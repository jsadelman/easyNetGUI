#ifndef INPUTCMDLINE_H
#define INPUTCMDLINE_H

#include <QLineEdit>

class InputCmdLine : public QLineEdit
{
    Q_OBJECT

public:
    InputCmdLine(QWidget *parent = 0);

public slots:
    void sendCommand();

signals:
    void commandReady(const QString & command);

};

#endif // INPUTCMDLINE_H
