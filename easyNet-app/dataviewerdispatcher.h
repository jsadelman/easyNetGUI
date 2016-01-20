#ifndef DATAVIEWERDISPATCHER_H
#define DATAVIEWERDISPATCHER_H


#include <QMap>
#include <QObject>
#include <QSharedPointer>

#include "dataviewer.h"



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
    QString getTrial(QString item);
    QString getRunMode(QString item);
    QString getResults(QString item);

    int dispatchModeOverride;
    bool dispatchModeAuto;
    QMap<QString, int> dispatchDefaultMode;
    QMap<QPair<int, int>, int> dispatchModeFST; // <previous mode, current mode> -> action

protected:
    struct TrialRunInfo
    {
        TrialRunInfo(QSharedPointer<QDomDocument> info);
        QString results;
        QString trial;
        QString runMode;
    };
    DataViewer *hostDataViewer;
    int previousDispatchMode;
    QMap <QString, QSharedPointer<QDomDocument> > trialRunInfoMap;


};

#endif // DATAVIEWERDISPATCHER_H
