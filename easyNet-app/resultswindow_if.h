#ifndef RESULTSWINDOW_IF_H
#define RESULTSWINDOW_IF_H

#include <QMainWindow>
#include <QMap>
#include <QDebug>

class QDomDocument;

class QSignalMapper;
class QActionGroup;



class ResultsWindow_If : public QMainWindow
{
    Q_OBJECT
public:

    explicit ResultsWindow_If(QWidget *parent = 0);
    virtual ~ResultsWindow_If();

public slots:
    void dispatch();


protected slots:

    void setSingleTrialMode(int mode) {singleTrialDispatchMode = mode; qDebug()
                << "setSingleTrialMode" << singleTrialDispatchMode;}
    void setTrialListMode(int mode) {trialListDispatchMode = mode;}
    virtual void open()=0;
    virtual void save()=0;
    virtual void copy()=0;


protected:
    enum {New=0, Overwrite, Append, MAX_DISPATCH_MODE};
    virtual void createActions();
    virtual void createMenus();
    virtual void createToolBars();
    virtual void dispatch_Impl(QDomDocument *info)=0;


    QList<QAction *> setSingleTrialDispatchModeActs;
    QList<QAction *> setTrialListDispatchModeActs;
    QSignalMapper *setSignleTrialDispatchModeMapper;
    QSignalMapper *setTrialListDispatchModeMapper;
    QActionGroup *setSignleTrialDispatchModeActGrouop;
    QActionGroup *setTrialListDispatchModeActGrouop;

    QMap<int, QString> dispatchModeName;
    int singleTrialDispatchMode;
    int trialListDispatchMode;
    QMap<QPair<int,int>, int> dispatchFST;

    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *copyAct;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QMenu   *fileMenu;
    QMenu   *editMenu;
    QMenu   *settingsMenu;
    QMenu   *setSingleTrialDispatchModeMenu;
    QMenu   *setTrialListDispatchModeMenu;


};

#endif // RESULTSWINDOW_IF_H
