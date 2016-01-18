#ifndef DATAVIEWER_H
#define DATAVIEWER_H

#include <QWidget>

class Ui_DataViewer;
class DataViewerDispatcher;
class ObjectCacheFilter;

class QDomDocument;


class DataViewer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool lazy READ isLazy WRITE setLazy NOTIFY lazyChanged)
public:
    DataViewer(Ui_DataViewer *ui, QWidget * parent = 0);
    virtual ~DataViewer();
    void setDispatcher(DataViewerDispatcher *dataViewerDispatcher);
    virtual bool contains(QString name)=0;
    bool isLazy() {return m_lazy;}
    void setLazy (bool lazy) {m_lazy = lazy; emit lazyChanged(lazy);}
    void setDefaultOpenDir(QString dir) {defaultOpenDir = dir;}
    void setDefaultSaveDir(QString dir) {defaultSaveDir = dir;}
    void setDefaultDir(QString dir);

public slots:
    virtual void addItem(QString name="", bool setCurrent=false)=0;
    void preDispatch(QDomDocument *info);
    virtual void dispatch();
    void setDispatchModeOverride(int mode);
    void setDispatchModeAuto(bool isAuto);


protected slots:
    void setUi();
    virtual void open()=0;
    virtual void save()=0;
    virtual void copy()=0;
    virtual void initiateRemoveItem(QString name)=0;
    virtual void removeItem(QString name)=0;
    virtual void updateCurrentItem(QString name);
    virtual void enableActions(bool enable);

signals:
    void lazyChanged(bool);

protected:

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
