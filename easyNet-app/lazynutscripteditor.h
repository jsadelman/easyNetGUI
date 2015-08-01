#ifndef LAZYNUTSCRIPTEDITOR_H
#define LAZYNUTSCRIPTEDITOR_H

#include <QPlainTextEdit>
#include <QMessageBox>

class LazyNutScriptEditor: public QPlainTextEdit
{
    Q_OBJECT

public:
    LazyNutScriptEditor(QWidget *parent = 0);
    ~LazyNutScriptEditor();

public slots:
    QStringList getSelectedText();
    QStringList getAllText();

private slots:
    void loadFile(const QString &fileName);


};
#endif // LAZYNUTSCRIPTEDITOR_H
