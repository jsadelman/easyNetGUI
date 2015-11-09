#ifndef TABLEWINDOW_H
#define TABLEWINDOW_H

#include "resultswindow_if.h"

class QSignalMapper;
class QActionGroup;
class TabsTableWidget;
class ObjectCacheFilter;
class QScrollArea;


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
    void preparePlot();
    void dataframeMerge();
    void addTable();
    void removeTable(QString name);
    virtual void setInfoVisible(bool visible) Q_DECL_OVERRIDE;
    virtual void refreshInfo() Q_DECL_OVERRIDE;

signals:
    void createNewRPlot(QString, QString, QMap<QString, QString>, QMap<QString, QString>, int);
    void showPlotSettings();

protected:
    virtual void createActions() Q_DECL_OVERRIDE;
    virtual void createToolBars() Q_DECL_OVERRIDE;
    virtual void dispatch_Impl(QDomDocument *info) Q_DECL_OVERRIDE;
    void showInfo(QString name);
    void hideInfo();
    QString newTableName();

    QMap <QString, int> dispatchModeMap; // <Table, Mode>
    QMap <QString, QSet<QString> > dispatchMap; // <results df, set of Tables>
    QMap <QString, QList<QDomDocument*> > trialRunInfoMap; // <Table, list of info XML>

    QAction *copyDFAct;
    QAction *dataframeMergeAct;
    QAction *findAct;
    QAction *plotAct;


    QAction *refreshAct;

    TabsTableWidget *tableWidget;

    int tableCounter;
    ObjectCacheFilter *dataframeFilter;

};

#endif // TABLEWINDOW_H
