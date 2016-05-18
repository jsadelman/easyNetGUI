#ifndef UI_DATAVIEWER_H
#define UI_DATAVIEWER_H

#include <QMainWindow>
#include <QMap>
#include <QDebug>

class DataViewer;
class QSignalMapper;
class QActionGroup;
class ObjectCacheFilter;
class ObjectUpdater;
class QVBoxLayout;

class WidgetFwdResizeEvent : public QWidget
{
    Q_OBJECT
public:
    WidgetFwdResizeEvent(QWidget *parent =0): QWidget(parent) {}
signals:
    void resizeEventOccured(QResizeEvent * event);
protected:
    virtual void resizeEvent(QResizeEvent * event) Q_DECL_OVERRIDE {emit resizeEventOccured(event);}
};

class Ui_DataViewer: public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(bool usePrettyNames READ usePrettyNames WRITE setUsePrettyNames NOTIFY usePrettyNamesChanged)
public:
    Ui_DataViewer();
    virtual ~Ui_DataViewer();
    virtual void setupUi(DataViewer *dataViewer);
    virtual QString currentItemName()=0;
    virtual QWidget *currentView()=0;
    bool contains(QString name) {return viewMap.contains(name);}
    QWidget *view(QString name) {return viewMap.value(name, nullptr);}
    bool usePrettyNames() {return m_usePrettyNames;}
    void setUsePrettyNames(bool prettyNamesEnabled) {m_usePrettyNames = prettyNamesEnabled; emit usePrettyNamesChanged(prettyNamesEnabled);}
    void addHistoryWidget(QDockWidget *historyWidget);

    QAction *openAct;
    QAction *saveAct;
    QAction *copyAct;
    QAction *destroyAct;
    QAction *settingsAct;
    QAction *historyAct;
    bool setup;

//    QAction *infoAct;

//    QList<QAction *> setDispatchModeOverrideActs;
//    QSignalMapper *setDispatchModeOverrideMapper;
//    QActionGroup *setDispatchModeOverrideActGroup;
//    QAction *setDispatchModeAutoAct;


//    QMap<int, QString> dispatchModeName;
//    QMap<int, QString> dispatchModeIconName;

    QMap<DataViewer*,QToolBar*> fileToolBar;
    QMap<DataViewer*,QToolBar*>editToolBar;
    QMap<DataViewer*,QToolBar*> dispatchToolBar;
    QToolBar *historyToolBar;
    QMap<DataViewer*,QList<QAction*> > actionSet;
    WidgetFwdResizeEvent *mainWidget;
//    QToolBar *infoToolBar;


    void setToolBars(DataViewer *dv);
public slots:
    virtual void addView(QString name, QWidget *view)=0;
    virtual QWidget *takeView(QString name)=0;
    virtual void setCurrentItem(QString name)=0;

signals:
    void deleteItemRequested(QString);
    void currentItemChanged(QString);
    void usePrettyNamesChanged(bool);


protected:


    virtual void createActions();
    virtual void createToolBars(DataViewer*);
    virtual void createViewer()=0;
    virtual void displayPrettyName(QString name)=0;
    QMap<QString, QWidget *> viewMap;
    ObjectCacheFilter *itemDescriptionFilter;
    ObjectUpdater *itemDescriptionUpdater;
    bool m_usePrettyNames;
    QMap<QString, QString> prettyName;
    QVBoxLayout *mainLayout;
    QToolBar* currentFileToolBar;
    QToolBar* currentEditToolBar;
    QToolBar* currentDispatchToolBar;
    QList<QDockWidget *> historyWidgets;

};

#endif // UI_DATAVIEWER_H
