#ifndef TABLEWINDOW_H
#define TABLEWINDOW_H

#include "resultswindow_if.h"

class QSignalMapper;
class QActionGroup;
class TabsTableWidget;
class ObjectCacheFilter;


class TableWindow: public ResultsWindow_If
{
    Q_OBJECT

public:
    explicit TableWindow(QWidget *parent = 0);
    ~TableWindow();

protected slots:
    virtual void open() Q_DECL_OVERRIDE;
    virtual void save() Q_DECL_OVERRIDE;
    virtual void copy() Q_DECL_OVERRIDE;
    void addTable();
    void removeTable(QString name);

protected:
    virtual void createActions() Q_DECL_OVERRIDE;
    virtual void createMenus() Q_DECL_OVERRIDE;
    virtual void createToolBars() Q_DECL_OVERRIDE;
    virtual void dispatch_Impl(QDomDocument *info) Q_DECL_OVERRIDE;
    QString newTableName();

    QMap <QString, int> dispatchModeMap;
    QMap <QString, QString> dispatchMap;
    QMap <QString, QList<QDomDocument*> > trialRunInfoMap;

    QAction *copyDFAct;
    QAction *mergeDFAct;
    QAction *findAct;
    QAction *plotAct;
    QAction *refreshAct;

    TabsTableWidget *tableWidget;
    int tableCounter;
    ObjectCacheFilter *dataframeFilter;
};

#endif // TABLEWINDOW_H
