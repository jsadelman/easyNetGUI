#ifndef DATAVIEWER_H
#define DATAVIEWER_H

#include <QWidget>
#include <QSharedPointer>
#include <QMap>

class Ui_DataViewer;
class DataViewerDispatcher;
class ObjectCacheFilter;
class ObjectUpdater;


class QDomDocument;


class DataViewer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool lazy READ isLazy WRITE setLazy NOTIFY lazyChanged)
    friend class DataViewerDispatcher;
public:
    DataViewer(Ui_DataViewer *ui, QWidget * parent = 0);
    virtual ~DataViewer();
    void setDispatcher(DataViewerDispatcher *dataViewerDispatcher);
    bool contains(QString name);
    bool isLazy() {return m_lazy;}
    void setLazy (bool lazy) {m_lazy = lazy; emit lazyChanged(lazy);}
    void setDefaultOpenDir(QString dir) {defaultOpenDir = dir;}
    void setDefaultSaveDir(QString dir) {defaultSaveDir = dir;}
    void setDefaultDir(QString dir);
    void addView(QString name);
    void removeView(QString name);
    QString currentItemName();

public slots:
    void addItem(QString name="", bool isBackup=false);
    virtual void addRequestedItem(QString name="", bool isBackup=false) =0;
    void preDispatch(QSharedPointer<QDomDocument> info);
    virtual void dispatch();
    void setDispatchModeOverride(int mode);
    void setDispatchModeAuto(bool isAuto);
    virtual void open()=0;
    virtual void save()=0;
    virtual void copy()=0;
    virtual void destroySelectedItems();
    virtual void snapshot(QString name="")=0;


protected slots:
    void setUi();
    void execAddItem(QDomDocument *domDoc, QString name);
    void initiateDestroyItem(QString name);
    virtual void destroyItem(QString name);
    virtual void destroyItem_impl(QString name)=0;
    virtual void setCurrentItem(QString name);
    virtual void enableActions(bool enable);
//    void setTrialRunInfo(QString item, QSharedPointer<QDomDocument> info);
    void setTrialRunMode(int mode);


signals:
    void lazyChanged(bool);
    void sendTrialRunInfo(QString, QSharedPointer<QDomDocument>);
    void itemRemoved(QString);
    void showSettingsRequested();
    void currentItemChanged(QString); // old setPlotSettings


protected:
    virtual void addItem_impl(QString name) {Q_UNUSED(name)}
    virtual QWidget* makeView(QString name) = 0;
    virtual void addNameToFilter(QString name) = 0;
    virtual void removeNameFromFilter(QString name) = 0;
    virtual void setNameInFilter(QString name) = 0;

//    QMap<QString, QWidget*> viewMap;
    Ui_DataViewer *ui;
    DataViewerDispatcher *dispatcher;
    ObjectCacheFilter *descriptionFilter;
    ObjectUpdater *descriptionUpdater;

    QString lastOpenDir;
    QString defaultOpenDir;
    QString lastSaveDir;
    QString defaultSaveDir;
    bool m_lazy;
    QMap<QString, bool> isBackupMap;
};

#endif // DATAVIEWER_H
