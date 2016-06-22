#ifndef DATAFRAMEVIEWERDISPATCHER_H
#define DATAFRAMEVIEWERDISPATCHER_H

#include "dataviewerdispatcher.h"

#include <QObject>

class DataframeViewer;
class QToolButton;
class QMenu;

class DataframeViewerDispatcher : public DataViewerDispatcher
{
    Q_OBJECT
public:
    DataframeViewerDispatcher(DataframeViewer *host);
    virtual ~DataframeViewerDispatcher();
    virtual void preDispatch(QSharedPointer<QDomDocument> info) Q_DECL_OVERRIDE;
    virtual void dispatch(QSharedPointer<QDomDocument> info) Q_DECL_OVERRIDE;

    QAction *clearMenuAct;

public slots:
    virtual void enableActions(bool enable);

protected:
    virtual QDomDocument *makePreferencesDomDoc() Q_DECL_OVERRIDE;

protected slots:
    void backup(QString name = "");
    void clear(QString name = "");
    void backupAndClear(QString name = "");
    virtual void addExtraActions();

private:
    DataframeViewer *host;
    QMap<QString, int> previousDispatchModeMap;
    QToolButton *clearButton;
    QMenu *clearMenu;
    QAction *clearAct;
    QAction *backupAndClearAct;
    QAction *mergeAct;

};

#endif // DATAFRAMEVIEWERDISPATCHER_H
