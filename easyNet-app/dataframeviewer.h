#ifndef DATAFRAMEVIEWER_H
#define DATAFRAMEVIEWER_H

#include "dataviewer.h"

#include <QMap>
#include <QMultiMap>

class DataFrameModel;
class PrettyHeadersModel;
class QTableView;
class ObjectCacheFilter;
class ObjectUpdater;
class QDomDocument;

class DataframeViewer : public DataViewer
{
    Q_OBJECT
public:
    DataframeViewer(Ui_DataViewer *ui, QWidget * parent = 0);
    virtual bool contains(QString name) Q_DECL_OVERRIDE;

public slots:
    virtual void addItem(QString item="") Q_DECL_OVERRIDE;
    void setPrettyHeadersForTrial(QString trial, QString df);
    virtual void dispatch();


protected slots:
    virtual void open();
    virtual void save();
    virtual void copy();
    virtual void removeItem(QString name);
    virtual void enableActions(bool enable) Q_DECL_OVERRIDE;
//    virtual void updateCurrentItem(QString name) Q_DECL_OVERRIDE;


    void updateDataframe(QDomDocument* domDoc, QString name);

protected:

    QMap<QString, DataFrameModel*> modelMap;
    QMultiMap<QString, QTableView*> viewsMap;
    QMap<QString, PrettyHeadersModel*> prettyHeadersModelMap;
    ObjectCacheFilter *dataframeFilter;
    ObjectUpdater *dataframeUpdater;

};

#endif // DATAFRAMEVIEWER_H
