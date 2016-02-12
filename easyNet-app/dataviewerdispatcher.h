#ifndef DATAVIEWERDISPATCHER_H
#define DATAVIEWERDISPATCHER_H


#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include <QModelIndex>

#include "dataviewer.h"
#include "enumclasses.h"

class HistoryTreeModel;
class HistoryWidget;
class QAction;
class QDomDocument;
class QDockWidget;
class QScrollArea;

class DataViewerDispatcher: public QObject
{
    Q_OBJECT
public:
    DataViewerDispatcher(DataViewer *host);
    virtual ~DataViewerDispatcher();
    virtual void preDispatch(QSharedPointer<QDomDocument> info)=0;
    virtual void dispatch(QSharedPointer<QDomDocument> info)=0;
    void setSingleTrialMode(int mode)   {dispatchDefaultMode.insert(trialRunModeName.value(TrialRunMode_Single), mode);}
    void setTrialListMode(int mode)     {dispatchDefaultMode.insert(trialRunModeName.value(TrialRunMode_List), mode);}
    void setTrialRunInfo(QString item, QList<QSharedPointer<QDomDocument> > info);
    void setTrialRunInfo(QString item, QSharedPointer<QDomDocument> info);
    void appendTrialRunInfo(QString item, QSharedPointer<QDomDocument> info);
    void copyTrialRunInfo(QString fromItem, QString toItem);
    QString trial(QString name);
    QString runMode(QString name);
    QString results(QString name);
    QList<QSharedPointer<QDomDocument> > info(QString name) {return trialRunInfoMap.value(name);}
    QList<QVariant> infoVariantList(QString name);
    void addToHistory(QString name, bool inView=false, QList<QSharedPointer<QDomDocument> > info=QList<QSharedPointer<QDomDocument> >());
    void removeFromHistory(QString name);
    bool inHistory(QString name);
    void setInView(QString name, bool inView);
    void setTrialRunMode(int mode);
    void restoreOverrideDefaultValue();

    QAction *historyAct;
    QAction *infoAct;
    int dispatchModeOverride;
    bool dispatchModeAuto;
    QMap<QString, int> dispatchDefaultMode;
    QMap<QPair<int, int>, int> dispatchModeFST; // <previous mode, current mode> -> action
    int previousDispatchOverrideMode;


public slots:
    void destroySelectedItems();

protected slots:
    void updateView(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles);
    void updateHistory(QString item, QSharedPointer<QDomDocument> info);
    void showInfo(bool show);

protected:
    void createHistory();
    void createInfo();

    struct TrialRunInfo
    {
        TrialRunInfo(QSharedPointer<QDomDocument> info);
        QString results;
        QString trial;
        QString runMode;
    };
    const QString no_trial = "<no-trial>";
    DataViewer *hostDataViewer;
    int previousDispatchMode;
    int currentDispatchAction;
    // preferred over QMultiMap since order of values is guaranteed and easy to clear QLists
    QMap <QString, QList<QSharedPointer<QDomDocument> > > trialRunInfoMap;
    HistoryTreeModel *historyModel;
    HistoryWidget  *historyWidget;
    QString previousItem;
    bool infoIsVisible;
    int trialRunMode;
    QDockWidget  *infoDock;
    QScrollArea *infoScroll;

};

#endif // DATAVIEWERDISPATCHER_H
