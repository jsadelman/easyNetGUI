#ifndef DATAVIEWERDISPATCHER_H
#define DATAVIEWERDISPATCHER_H


#include <QMap>
#include <QObject>
#include <QSharedPointer>

#include "dataviewer.h"

class CheckListModel;
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
    virtual void dispatch(QSharedPointer<QDomDocument> info);
    void setSingleTrialMode(int mode) {dispatchDefaultMode.insert("single", mode);}
    void setTrialListMode(int mode) {dispatchDefaultMode.insert("list", mode);}
    void setTrialRunInfo(QString item, QSharedPointer<QDomDocument> info);
    void copyTrialRunInfo(QString fromItem, QString toItem);
    QString getTrial(QString name);
    QString getRunMode(QString name);
    QString getResults(QString name);
    void addToHistory(QString name);
    void moveFromViewerToHistory(QString name);
    bool inHistory(QString name);

    QAction *historyAct;
    int dispatchModeOverride;
    bool dispatchModeAuto;
    QMap<QString, int> dispatchDefaultMode;
    QMap<QPair<int, int>, int> dispatchModeFST; // <previous mode, current mode> -> action

protected slots:
    void moveFromHistoryToViewer();

    void removeFromHistory();
    void displayItemFromHistory(QString name);
    void setHistoryVisible(bool visible);


protected:
    struct TrialRunInfo
    {
        TrialRunInfo(QSharedPointer<QDomDocument> info);
        QString results;
        QString trial;
        QString runMode;
    };
    void removePreviousItem();
    DataViewer *hostDataViewer;
    int previousDispatchMode;
    QMap <QString, QSharedPointer<QDomDocument> > trialRunInfoMap;
    CheckListModel *historyModel;
    HistoryWidget  *historyWidget;
    QString previousItem;

};

#endif // DATAVIEWERDISPATCHER_H
