#ifndef LAZYNUTMACRO_H
#define LAZYNUTMACRO_H

#include <QStateMachine>
#include <QFinalState>


class LazyNutJob;

class LazyNutMacro : public QStateMachine
{
    Q_OBJECT
public:
    explicit LazyNutMacro(QObject *parent = 0);
    QFinalState *endOfMacro;

signals:
    // signal CommandSequencer::commandsExecuted() gets connected to signal next()
    // when executing SessionManager::submitMacro(LazyNutMacro *macro)
    void next();

private slots:

    void macroStarted();
    void macroEnded();


};

#endif // LAZYNUTMACRO_H
