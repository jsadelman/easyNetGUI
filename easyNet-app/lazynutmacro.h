#ifndef LAZYNUTMACRO_H
#define LAZYNUTMACRO_H

#include <QStateMachine>
#include <QFinalState>

class MacroQueue;

class LazyNutJob_DEPRECATED;

class LazyNutMacro : public QStateMachine
{
    Q_OBJECT
public:
    explicit LazyNutMacro(MacroQueue* queue, QObject *parent = 0);
    QFinalState *endOfMacro;

signals:
    // signal CommandSequencer::commandsExecuted() gets connected to signal next()
    // when executing SessionManager::submitMacro(LazyNutMacro *macro)
    void next();

private slots:

    void macroStarted();
    void macroEnded();

private:
    MacroQueue* macroQueue;

};

#endif // LAZYNUTMACRO_H
