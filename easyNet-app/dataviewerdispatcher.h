#ifndef DATAVIEWERDISPATCHER_H
#define DATAVIEWERDISPATCHER_H


#include <QMap>
#include <QObject>

#include "dataviewer.h"



class QDomDocument;

class DataViewerDispatcher: public QObject
{
    Q_OBJECT
public:
    DataViewerDispatcher(DataViewer *host);
    virtual ~DataViewerDispatcher();
    virtual void preDispatch(QDomDocument *info)=0;
    virtual void dispatch(QDomDocument *info)=0;
    void setSingleTrialMode(int mode) {dispatchDefaultMode.insert("single", mode);}
    void setTrialListMode(int mode) {dispatchDefaultMode.insert("list", mode);}

    int dispatchModeOverride;
    bool dispatchModeAuto;
    QMap<QString, int> dispatchDefaultMode;
    QMap<QPair<int, int>, int> dispatchModeFST; // <previous mode, current mode> -> action

protected:
    struct TrialRunInfo
    {
        TrialRunInfo(QDomDocument *info);
        QString results;
        QString trial;
        QString runMode;
    };
    DataViewer *hostDataViewer;
    int previousDispatchMode;
    QMap <QString, QDomDocument*> trialRunInfoMap;


};

#endif // DATAVIEWERDISPATCHER_H
