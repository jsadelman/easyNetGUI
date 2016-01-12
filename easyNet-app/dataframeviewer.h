#ifndef DATAFRAMEVIEWER_H
#define DATAFRAMEVIEWER_H

#include "dataviewer.h"

class DataFrameModel;
class PrettyHeadersModel;
class QTableView;
class ObjectCacheFilter;
class ObjectUpdater;

class DataframeViewer : public DataViewer
{
    Q_OBJECT
public:
    DataframeViewer(QWidget * parent = 0);
    ~DataframeViewer();


protected slots:
    virtual void open();
    virtual void save();
    virtual void copy();
    virtual void removeItem(QString name);
    virtual void updateCurrentItem(QString name);


    void addDataframe();
    void updateDataframe(QDomDocument* domDoc, QString name);
    void setPrettyHeadersForTrial(QString trial, QString df);

    QMap<QString, DataFrameModel*> modelMap;
    QMultiMap<QString, QTableView*> viewsMap;
    QMap<QString, PrettyHeadersModel*> prettyHeadersModelMap;
    ObjectCacheFilter *dataframeFilter;
    ObjectUpdater *dataframeUpdater;

    QString lastSaveDir;
};

#endif // DATAFRAMEVIEWER_H
