#ifndef MACROSTATE_H
#define MACROSTATE_H

#include <QStateMachine>

class MacroState : public QStateMachine
{
    Q_OBJECT
public:
    explicit MacroState(QObject *parent = 0);


signals:

public slots:

private:
    QStringList stringList;

};

#endif // MACROSTATE_H
