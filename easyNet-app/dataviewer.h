#ifndef DATAVIEWER_H
#define DATAVIEWER_H

#include <QWidget>
#include <QSharedPointer>
#include <QMap>
#include <QSet>

#include "defaultdirs.h"

class Ui_DataViewer;
class DataViewerDispatcher;
class ObjectCacheFilter;
class ObjectUpdater;


class QDomDocument;


class DataViewer : public QWidget, public DefaultDirs
{
    Q_OBJECT
    Q_PROPERTY(bool lazy READ isLazy WRITE setLazy NOTIFY lazyChanged)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString itemPrettyName READ itemPrettyName WRITE setItemPrettyName)

    friend class DataViewerDispatcher;
public:
    DataViewer(Ui_DataViewer *ui, QWidget * parent = 0);
    virtual ~DataViewer();
    void setDispatcher(DataViewerDispatcher *dataViewerDispatcher);
    bool contains(QString name);
    bool isLazy() {return m_lazy;}
    void setLazy (bool lazy) {m_lazy = lazy; emit lazyChanged(lazy);}
    void addView(QString name);
    void removeView(QString name);
    QString currentItemName();
    QStringList items() {return m_items.toList();}
    QString name() {return m_name;}
    void setName(QString txt) {m_name = txt;}
    QString itemPrettyName() {return m_itemPrettyName;}
    void setItemPrettyName(QString txt) {m_itemPrettyName = txt;}
    bool isHidden(QString name) {return hiddenItems.contains(name);}
    int viewState(QString name) {return viewStateMap.value(name, -1);}
    void setViewState(QString name, int state) {viewStateMap[name] = state;
                                                emit viewStateChanged(name, state);}
    void resetViewStates();
    void updateDependeesViewStates(QString depender);

public slots:
    void addItem(QString name="", bool hidden=false);
    virtual void addRequestedItem(QString name="", bool isBackup=false) =0;
    void preDispatch(QSharedPointer<QDomDocument> info);
    virtual void dispatch();
    virtual void open()=0;
    virtual void save()=0;
    virtual void copy()=0;
    virtual void destroySelectedItems();
    virtual void snapshot(QString name="", QString snapshotName="")=0;
    virtual bool setCurrentItem(QString name);


protected slots:
    void setUi();
    void execAddItem(QDomDocument *domDoc, QString name);
    void initiateDestroyItem(QString name);
    virtual void destroyItem(QString name);
    virtual void destroyItem_impl(QString name)=0;
    virtual void enableActions(bool enable);
    void setTrialRunMode(int mode);


signals:
    void lazyChanged(bool);
    void sendTrialRunInfo(QString, QSharedPointer<QDomDocument>);
    void itemRemoved(QString);
    void showSettingsRequested();
    void currentItemChanged(QString); // old setPlotSettings
    void viewStateChanged(QString, int);


protected:
    virtual void addItem_impl(QString name) {Q_UNUSED(name)}
    virtual void setFirstViewState(QString name) = 0;
    virtual QWidget* makeView(QString name) = 0;
    virtual void addNameToFilter(QString name) = 0;
    virtual void removeNameFromFilter(QString name) = 0;
    virtual void setNameInFilter(QString name) = 0;

    Ui_DataViewer *ui;
    DataViewerDispatcher *dispatcher;
    ObjectCacheFilter *descriptionFilter;
    ObjectUpdater *descriptionUpdater;

    bool m_lazy;
    QSet<QString> m_items;
    QSet<QString> hiddenItems;
    QString m_name;
    QString m_itemPrettyName;
    QMap<QString, int> viewStateMap;
};

#endif // DATAVIEWER_H
