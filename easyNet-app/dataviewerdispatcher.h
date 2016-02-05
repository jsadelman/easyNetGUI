#ifndef DATAVIEWERDISPATCHER_H
#define DATAVIEWERDISPATCHER_H


#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include <QModelIndex>

#include "dataviewer.h"

class HistoryTreeModel;
class HistoryWidget;
class QAction;
class QDomDocument;

class DataViewerDispatcher: public QObject
{
    Q_OBJECT
public:
    DataViewerDispatcher(DataViewer *host);
    virtual ~DataViewerDispatcher();
    virtual void preDispatch(QSharedPointer<QDomDocument> info)=0;
    virtual void dispatch(QSharedPointer<QDomDocument> info)=0;
    void setSingleTrialMode(int mode) {dispatchDefaultMode.insert("single", mode);}
    void setTrialListMode(int mode) {dispatchDefaultMode.insert("list", mode);}
    void setTrialRunInfo(QString item, QSharedPointer<QDomDocument> info);
    void copyTrialRunInfo(QString fromItem, QString toItem);
    QString trial(QString name);
    QString runMode(QString name);
    QString results(QString name);
    QSharedPointer<QDomDocument> info(QString name) {return trialRunInfoMap.value(name);}
    void addToHistory(QString name, bool inView=false, QSharedPointer<QDomDocument> info=QSharedPointer<QDomDocument>());
//    void moveFromViewerToHistory(QString name);
    bool inHistory(QString name);
    void setInView(QString name, bool inView);

    QAction *historyAct;
    int dispatchModeOverride;
    bool dispatchModeAuto;
    QMap<QString, int> dispatchDefaultMode;
    QMap<QPair<int, int>, int> dispatchModeFST; // <previous mode, current mode> -> action

protected slots:
//    void moveFromHistoryToViewer();

    void removeFromHistory();
//    void displayItemFromHistory(QString name);
    void setHistoryVisible(bool visible);
    void updateView(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles);
    void updateHistory(QString item, QSharedPointer<QDomDocument> info);

protected:
    struct TrialRunInfo
    {
        TrialRunInfo(QSharedPointer<QDomDocument> info);
        QString results;
        QString trial;
        QString runMode;
    };
    const QString no_trial = "<no-trial>";
//    void removePreviousItem();
    DataViewer *hostDataViewer;
    int previousDispatchMode;
    QMap <QString, QSharedPointer<QDomDocument> > trialRunInfoMap;
    HistoryTreeModel *historyModel;
    HistoryWidget  *historyWidget;
    QString previousItem;
    bool no_update_view;

};

#endif // DATAVIEWERDISPATCHER_H
