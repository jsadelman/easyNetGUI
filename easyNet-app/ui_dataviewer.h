#ifndef UI_DATAVIEWER_H
#define UI_DATAVIEWER_H

#include <QMainWindow>
#include <QMap>

class DataViewer;
class QSignalMapper;
class QActionGroup;
class ObjectCacheFilter;
class ObjectUpdater;

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

    QAction *openAct;
    QAction *saveAct;
    QAction *copyAct;
    QAction *destroyAct;


//    QAction *infoAct;

    QList<QAction *> setDispatchModeOverrideActs;
    QSignalMapper *setDispatchModeOverrideMapper;
    QActionGroup *setDispatchModeOverrideActGroup;
    QAction *setDispatchModeAutoAct;


    QMap<int, QString> dispatchModeName;
    QMap<int, QString> dispatchModeIconName;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *dispatchToolBar;
//    QToolBar *infoToolBar;


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
    virtual void createToolBars();
    virtual void createViewer()=0;
    virtual void displayPrettyName(QString name)=0;
    QMap<QString, QWidget *> viewMap;
    ObjectCacheFilter *itemDescriptionFilter;
    ObjectUpdater *itemDescriptionUpdater;
    bool m_usePrettyNames;
    QMap<QString, QString> prettyName;

};

#endif // UI_DATAVIEWER_H
