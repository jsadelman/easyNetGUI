#ifndef RESULTSWINDOW_IF_H
#define RESULTSWINDOW_IF_H

#include <QMainWindow>
#include <QMap>
#include <QDebug>

class QDomDocument;

class QSignalMapper;
class QActionGroup;
class QScrollArea;



class ResultsWindow_If : public QMainWindow
{
    Q_OBJECT
public:

    explicit ResultsWindow_If(QWidget *parent = 0);
    virtual ~ResultsWindow_If();

public slots:
    void dispatch();


protected slots:

    void setSingleTrialMode(int mode) {singleTrialDispatchMode = mode;}
    void setTrialListMode(int mode) {trialListDispatchMode = mode;}
    void setDispatchModeOverride(int mode);
    void setDispatchModeAuto(bool isAuto);
    virtual void open()=0;
    virtual void save()=0;
    virtual void copy()=0;
    virtual void setInfoVisible(bool visible)=0;
    virtual void refreshInfo()=0;
    virtual void preDispatch(QDomDocument *info)=0;


protected:
    virtual void createActions();
    virtual void createToolBars();
    virtual void dispatch_Impl(QDomDocument *info)=0;


    QList<QAction *> setSingleTrialDispatchModeActs;
    QList<QAction *> setTrialListDispatchModeActs;
    QList<QAction *> setDispatchModeOverrideActs;
    QSignalMapper *setSignleTrialDispatchModeMapper;
    QSignalMapper *setTrialListDispatchModeMapper;
    QSignalMapper *setDispatchModeOverrideMapper;
    QActionGroup *setSignleTrialDispatchModeActGrouop;
    QActionGroup *setTrialListDispatchModeActGrouop;
    QActionGroup *setDispatchModeOverrideActGroup;
    QAction *setDispatchModeAutoAct;

    int dispatchModeOverride;
    bool dispatchModeAuto;

    QMap<int, QString> dispatchModeName;
    QMap<int, QString> dispatchModeIconName;
    int singleTrialDispatchMode;
    int trialListDispatchMode;

    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *copyAct;
    QAction *infoAct;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *dispatchToolBar;
    QToolBar *infoToolBar;
    QMenu   *dispatchModeOverrideMenu;
    QMenu   *fileMenu;
    QMenu   *editMenu;
    QMenu   *settingsMenu;
    QMenu   *setSingleTrialDispatchModeMenu;
    QMenu   *setTrialListDispatchModeMenu;

    QDockWidget  *infoDock;
    QScrollArea  *infoScroll;
    bool infoVisible;


};

#endif // RESULTSWINDOW_IF_H