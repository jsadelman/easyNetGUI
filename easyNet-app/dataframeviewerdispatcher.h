#ifndef DATAFRAMEVIEWERDISPATCHER_H
#define DATAFRAMEVIEWERDISPATCHER_H

#include "dataviewerdispatcher.h"

#include <QObject>

class DataframeViewer;

class DataframeViewerDispatcher : public DataViewerDispatcher
{
    Q_OBJECT
public:
    DataframeViewerDispatcher(DataframeViewer *host);
    virtual ~DataframeViewerDispatcher();
    virtual void preDispatch(QSharedPointer<QDomDocument> info) Q_DECL_OVERRIDE;
    virtual void dispatch(QSharedPointer<QDomDocument> info) Q_DECL_OVERRIDE;

};

#endif // DATAFRAMEVIEWERDISPATCHER_H
