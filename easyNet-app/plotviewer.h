#ifndef PLOTVIEWER_H
#define PLOTVIEWER_H

#include "resultswindow_if.h"

#include <QMap>
#include <QTimer>

class QSvgWidget;
class QToolBar;
class QAction;
class QLabel;
class QByteArray;

class ObjectCacheFilter;


class PlotViewer: public ResultsWindow_If
{
    Q_OBJECT

public:
    PlotViewer(QString easyNetHome, QWidget *parent=0);
    ~PlotViewer();

    QTabWidget*     plotPanel;
    QSvgWidget *currentSvgWidget();



public slots:
    void updateAllActivePlots();
signals:
    void sendDrawCmd(QString);
    void showPlotSettings();
    void setPlot(QString);
    void resized(QSize);
    void hidePlotSettings();

protected slots:
    virtual void open() Q_DECL_OVERRIDE;
    virtual void save() Q_DECL_OVERRIDE;
    virtual void copy() Q_DECL_OVERRIDE;

protected:
    virtual void createActions() Q_DECL_OVERRIDE;
    virtual void createToolBars() Q_DECL_OVERRIDE;
    virtual void dispatch_Impl(QDomDocument *info) Q_DECL_OVERRIDE;

private slots:
    void loadByteArray(QString name, QByteArray byteArray);
    void addPlot(QString name, QString sourceDataframe="");
    void resizeTimeout();
    void snapshot();
    void currentTabChanged(int index);
    void freeze(QSvgWidget *svg = nullptr);
    void renamePlot();
    void deletePlot();
    void makeSnapshot(QString name);


private:
    void paintEvent(QPaintEvent * event);
    void resizeEvent(QResizeEvent*);
    void setPlotActive(bool isActive, QSvgWidget *svg = nullptr);
    void updateActivePlots();
    QSvgWidget * newSvg(QString name);
    QSvgWidget *snapshot(QSvgWidget *svg);

    QToolBar*       fileToolBar;
    QToolBar*       editToolBar;
    QToolBar*       navigationToolBar;
    QAction *       settingsAct;
    QAction *       refreshAct;
    QAction *       snapshotAct;
    QAction *       renameAct;
    QAction *       deleteAct;

    QString         easyNetHome;
    QLabel*         titleLabel;
    int             progressiveTabIdx;
    QMap <QSvgWidget*, QString> plotName;
    QMap <QSvgWidget*, bool> plotIsActive;
    QMap <QSvgWidget*, QByteArray> byteArray;
    QMap <QSvgWidget*, bool> plotIsUpToDate;
    QMap <QSvgWidget*, bool> plotSourceModified;
    QMap <QSvgWidget*, QList<QDomDocument*> > trialRunInfoMap; // <svg, list of info XML>
    QMap <QSvgWidget*, int> dispatchModeMap;
    QMap <QString, QSet<QString> > plotDataframeMap; // <dataframe, set of rplots>
    QTimer*         resizeTimer;
    bool            pend;
    ObjectCacheFilter *dataframeFilter;

};

#endif // PLOTVIEWER_H
