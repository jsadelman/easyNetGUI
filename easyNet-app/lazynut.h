#ifndef LAZYNUT_H
#define LAZYNUT_H

#include <QProcess>

class LazyNut: public QProcess
{
    Q_OBJECT

public:
    LazyNut(QObject *parent);
    ~LazyNut();
    void sendCommand(QString command);


public slots:
    void getOutput();

signals:
    void outputReady(QString);
};

#endif // LAZYNUT_H
