#ifndef DATAVIEWER_H
#define DATAVIEWER_H

#include <QWidget>
#include <QSharedPointer>
#include <QMap>

class Ui_DataViewer;
class DataViewerDispatcher;
class ObjectCacheFilter;

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
    QWidget *view(QString name) {return viewMap.value(name, nullptr);}

public slots:
    virtual void addItem(QString name="", bool setCurrent=false, bool isBackup=false);
    void preDispatch(QSharedPointer<QDomDocument> info);
    virtual void dispatch();
    void setDispatchModeOverride(int mode);
    void setDispatchModeAuto(bool isAuto);
    virtual void open()=0;
    virtual void save()=0;
    virtual void copy()=0;


protected slots:
    void setUi();

    virtual void initiateRemoveItem(QString name)=0;
    virtual void removeItem(QString name)=0;
    virtual void updateCurrentItem(QString name);
    virtual void enableActions(bool enable);
    void setTrialRunInfo(QString item, QSharedPointer<QDomDocument> info);

signals:
    void lazyChanged(bool);
    void sendTrialRunInfo(QString, QSharedPointer<QDomDocument>);

protected:
    virtual void addItem_impl(QString name) {Q_UNUSED(name)}
    virtual QWidget* makeView() = 0;
    virtual void addNameToFilter(QString name) = 0;
    virtual void removeNameFromFilter(QString name) = 0;
    virtual void setNameInFilter(QString name) = 0;

    QMap<QString, QWidget*> viewMap;
    Ui_DataViewer *ui;
    DataViewerDispatcher *dispatcher;
    ObjectCacheFilter *destroyedObjectsFilter;
    QString lastOpenDir;
    QString defaultOpenDir;
    QString lastSaveDir;
    QString defaultSaveDir;
    bool m_lazy;
};

#endif // DATAVIEWER_H
