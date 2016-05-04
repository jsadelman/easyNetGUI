#ifndef TRIALEDITOR_H
#define TRIALEDITOR_H

#include <QScrollArea>
#include <QMainWindow>

class ObjectCacheFilter;
class ObjectUpdater;
class QDomDocument;
class QVBoxLayout;
class TrialXML;
class QToolBar;
//class QWidgets;
//class QMainWindow;

class TrialEditor: public QMainWindow
{
    Q_OBJECT
public:
    TrialEditor(QWidget *parent = 0);
    ~TrialEditor();

public slots:
    void setTrialName(QString name);
signals:
    void loadTrialSignal();
private slots:
    void buildForm(QDomDocument* domDoc);

private:
    ObjectCacheFilter* trialFilter;
    ObjectUpdater* trialDescriptionUpdater;
    TrialXML* form;
    QVBoxLayout *layout;
    QToolBar* trialToolBar;
    QScrollArea* scrollArea;
    QWidget* dummy;
};

#endif // TRIALEDITOR_H
