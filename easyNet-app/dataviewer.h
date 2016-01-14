#ifndef DATAVIEWER_H
#define DATAVIEWER_H

#include <QWidget>

class Ui_DataViewer;
class DataViewerDispatcher;

class QDomDocument;


class DataViewer : public QWidget
{
    Q_OBJECT
public:
    DataViewer(Ui_DataViewer *ui, QWidget * parent = 0);
    virtual ~DataViewer();
    void setDispatcher(DataViewerDispatcher *dataViewerDispatcher);
    virtual bool contains(QString name)=0;
    virtual void addItem(QString item="")=0;

public slots:
    void preDispatch(QDomDocument *info);
    virtual void dispatch();
    void setDispatchModeOverride(int mode);
    void setDispatchModeAuto(bool isAuto);


protected slots:
    void setUi();
    virtual void open()=0;
    virtual void save()=0;
    virtual void copy()=0;
    virtual void removeItem(QString name)=0;
    virtual void updateCurrentItem(QString name);
    virtual void enableActions(bool enable);

protected:

    Ui_DataViewer *ui;
    DataViewerDispatcher *dispatcher;
    QString lastOpenDir;
    QString lastSaveDir;
};

#endif // DATAVIEWER_H
