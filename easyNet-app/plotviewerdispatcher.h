#ifndef PLOTVIEWERDISPATCHER_H
#define PLOTVIEWERDISPATCHER_H

#include "dataviewerdispatcher.h"

#include <QObject>

class PlotViewer;

class PlotViewerDispatcher : public DataViewerDispatcher
{
    Q_OBJECT
public:
    PlotViewerDispatcher(PlotViewer *host);
    ~PlotViewerDispatcher();
    virtual void preDispatch(QSharedPointer<QDomDocument> info) Q_DECL_OVERRIDE;
    virtual void dispatch(QSharedPointer<QDomDocument> info) Q_DECL_OVERRIDE;

private:
    QStringList affectedPlots(QString results);

    PlotViewer *host;
};

#endif // PLOTVIEWERDISPATCHER_H
