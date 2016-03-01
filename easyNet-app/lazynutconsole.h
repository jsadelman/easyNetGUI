#ifndef LAZYNUTCONSOLE_H
#define LAZYNUTCONSOLE_H

#include <QGroupBox>

class CmdOutput;
class InputCmdLine;

class LazyNutConsole: public QGroupBox
{
    Q_OBJECT

public:
    LazyNutConsole(QWidget *parent = 0);

public slots:
    void setConsoleFontSize(int size);
signals:
    void historyKey(int dir, QString text);

private slots:
    void showHistory(QString line);

private:
    CmdOutput* cmdOutput;
    InputCmdLine* inputCmdLine;

};

#endif // LAZYNUTCONSOLE_H
