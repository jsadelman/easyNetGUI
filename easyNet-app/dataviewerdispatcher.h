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
    Q_PROPERTY(bool snapshotActive READ snapshotActive WRITE setSnapshotActive)
    Q_PROPERTY(bool copyDfActive READ copyDfActive WRITE setCopyDfActive)

public:
    DataViewerDispatcher(DataViewer *host);
    virtual ~DataViewerDispatcher();
    virtual void preDispatch(QSharedPointer<QDomDocument> info)=0;
    virtual void dispatch(QSharedPointer<QDomDocument> info)=0;
    void setSingleTrialMode(int mode)   {dispatchDefaultMode.insert(trialRunModeName.value(TrialRunMode_Single), mode);}
    void setTrialListMode(int mode)     {dispatchDefaultMode.insert(trialRunModeName.value(TrialRunMode_List), mode);}
//    void setTrialRunInfo(QString item, QList<QSharedPointer<QDomDocument> > info);
//    void setTrialRunInfo(QString item, QSharedPointer<QDomDocument> info);
//    void appendTrialRunInfo(QString item, QSharedPointer<QDomDocument> info);
//    void copyTrialRunInfo(QString fromItem, QString toItem);
    QString trial(QString name);
    QString runMode(QString name);
    QString results(QString name);
//    QList<QSharedPointer<QDomDocument> > info(QString name) {return trialRunInfoMap.value(name);}
//    QList<QVariant> infoVariantList(QString name);
    void addToHistory(QString name, bool inView=false);
    void removeFromHistory(QString name);
    bool inHistory(QString name);
    bool isInView(QString name);
    void setInView(QString name, bool inView);
    void setTrialRunMode(int mode);
    bool snapshotActive() {return m_snapshotActive;}
    void setSnapshotActive(bool active) {m_snapshotActive = active;}
    bool copyDfActive() {return m_copyDfActive;}
    void setCopyDfActive(bool active) {m_copyDfActive = active;}

    QAction *infoAct;
    QAction *preferencesAct;
//    int dispatchModeOverride;
//    bool dispatchModeAuto;
    QMap<QString, int> dispatchDefaultMode;
    QMap<int, QString> dispatchModeText;
    QMap<QPair<int, int>, int> dispatchModeFST; // <previous mode, current mode> -> action


public slots:
    void destroySelectedItems();
    void updateView(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles);
    void updateHistory(QString name);
    void showInfo(bool show, QString name="");
    void updateInfo(QString name);

protected slots:
    void showPreferences();

protected:
    void createHistoryWidget();
    void createInfoWidget();
    virtual QDomDocument *makePreferencesDomDoc() = 0;

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
//    QMap <QString, QList<QSharedPointer<QDomDocument> > > trialRunInfoMap;
    HistoryTreeModel *historyModel;
    HistoryWidget  *historyWidget;
    QString previousItem;
    bool infoIsVisible;
    int trialRunMode;
    QDockWidget  *infoDock;
    QScrollArea *infoScroll;
    bool m_snapshotActive;
    bool m_copyDfActive;

};

#endif // DATAVIEWERDISPATCHER_H
