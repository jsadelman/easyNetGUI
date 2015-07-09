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

signals:
    void historyKey(int dir);

private slots:
    void showHistory(QString line);

private:
    CmdOutput* cmdOutput;
    InputCmdLine* inputCmdLine;

};

#endif // LAZYNUTCONSOLE_H
