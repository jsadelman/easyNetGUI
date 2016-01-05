#ifndef PLOTVIEWER_H
#define PLOTVIEWER_H

#include "resultswindow_if.h"

#include <QMap>
#include <QTimer>
#include <QDialog>

class QSvgWidget;
class QToolBar;
class QAction;
class QLabel;
class QByteArray;
class QScrollArea;
class ObjectNameValidator;
class ObjectUpdater;
class QMessageBox;
class QCheckBox;




class FullScreenSvgDialog: public QDialog
{
    Q_OBJECT
public:
    FullScreenSvgDialog(QWidget *parent = 0);
    void loadByteArray(QByteArray byteArray);
    void clearSvg();

private:
    QSvgWidget *svg;
};


class ObjectCacheFilter;


class PlotViewer: public ResultsWindow_If
{
    Q_OBJECT

public:
     enum {Tab_DefaultState = 0, Tab_Updating, Tab_Ready, Tab_Old}; // same as in TabsTableWidget
    PlotViewer(QString easyNetHome, QWidget *parent=0);
    ~PlotViewer();

    QSvgWidget *currentSvgWidget();
    void updateActionEnabledState(QSvgWidget* svg);
    QString uniqueName(QString name);
    QString plotType(QString name);
public slots:
    void updateAllActivePlots();
     void setTabState(int index, int state=Tab_DefaultState); // same as in TabsTableWidget
     void setCurrentPlot(QString name);
signals:
    void sendDrawCmd(QString);
    void showPlotSettings();
    void setPlot(QString);
    void resized(QSize);
    void hidePlotSettings();
    void createNewRPlot(QString, QString, QMap<QString, QString>, int);
    void removePlot(QString);

protected slots:
    virtual void open() Q_DECL_OVERRIDE;
    virtual void save() Q_DECL_OVERRIDE;
    virtual void copy() Q_DECL_OVERRIDE;
    virtual void setInfoVisible(bool visible) Q_DECL_OVERRIDE;
    virtual void refreshInfo() Q_DECL_OVERRIDE;
    void newRPlot(QString name);


    virtual void preDispatch(QDomDocument *info) Q_DECL_OVERRIDE;
protected:
    virtual void createActions() Q_DECL_OVERRIDE;
    virtual void createToolBars() Q_DECL_OVERRIDE;
    virtual void dispatch_Impl(QDomDocument *info) Q_DECL_OVERRIDE;
    void showInfo(QSvgWidget* svg);
    void hideInfo();

private slots:
    void loadByteArray(QString name, QByteArray byteArray);
    void resizeTimeout();
    void snapshot(QString name = QString());
    void currentTabChanged(int index);
    void freeze(QSvgWidget *svg = nullptr);
    void renamePlot();
    void deletePlot(QString name);
    void triggerPlotUpdate(QString name=QString());
    void setupFullScreen();
    void generatePrettyName(QString plotName, QString type, QDomDocument* domDoc);
    void addSourceDataframes(QStringList newDataframes=QStringList());
    void dfSourceModified(QString df);


private:
    void paintEvent(QPaintEvent * event);
    void resizeEvent(QResizeEvent*);
    void setSvgActive(bool isActive, QSvgWidget *svg = nullptr);
    void updateActivePlots();
    QString plotCloneName(QString name);
//    QString normalisedName(QString name);
    void renamePlot(QString oldName, QString newName = QString());


    QString cloneRPlot(QString name, QString newName=QString());
    QSvgWidget * newSvg(QString name);

    FullScreenSvgDialog *fullScreenSvgDialog;

    QAction *       settingsAct;
    QAction *       refreshAct;
    QAction *       snapshotAct;
    QAction *       renameAct;
    QAction *       fullScreenAct;

    QString         easyNetHome;
//    QLabel*         titleLabel;
    int             progressiveTabIdx;
    QMap <QString, QSvgWidget*> plotSvg;
//    QMap <QString, QString> plotType;
    QMap <QString, bool> anyTrialPlot;
    QMap <QString, QMap<QString, QString> > plotSourceDataframeSettings; // <rplot <key, val> >
    QMap <QSvgWidget*, bool> svgIsActive;
    QMap <QSvgWidget*, QByteArray> svgByteArray;
    QMap <QSvgWidget*, bool> svgIsUpToDate;
    QMap <QSvgWidget*, bool> svgSourceModified;
    QMap <QSvgWidget*, QDomDocument*> svgTrialRunInfo; // <svg, info XML>
    QMap <QSvgWidget*, int> svgDispatchOverride;
    QMultiMap <QString, QString> sourceDataframeOfPlots; // <dataframe, rplots>
    QMultiMap <QString, QString> dataframeMergeOfSource; // <dataframe, dataframe_merges>
    QStringList plotClones;

    QTimer*         resizeTimer;
    bool            pend;
    ObjectCacheFilter *dataframeFilter;
    ObjectCacheFilter *plotFilter;
    ObjectUpdater     *plotUpdater;
    QTabWidget*     plotPanel;
    bool            fullScreen;
    QSize           fullScreenSize;
    ObjectNameValidator *validator;
    QMessageBox *makeSnapshotMsg;
    QCheckBox *dontAskAgainMakeSnapshotCheckBox;
    bool askMakeSnapshot;
    bool makeSnapshot;
};

#endif // PLOTVIEWER_H
