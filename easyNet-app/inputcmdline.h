#ifndef INPUTCMDLINE_H
#define INPUTCMDLINE_H

#include <QLineEdit>
class Event;

class InputCmdLine : public QLineEdit
{
    Q_OBJECT

public:
    InputCmdLine(QWidget *parent = 0);

public slots:
    void sendCommand();

signals:
    void commandReady(const QString & command);
    void historyKey(int dir, QString text);

protected:
//    void changeEvent(QEvent *e);

    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // INPUTCMDLINE_H
