#ifndef PLOTVIEWER_H
#define PLOTVIEWER_H

#include "dataviewer.h"

#include <QMap>
#include <QDialog>
#include <QSharedPointer>

class QSvgWidget;
class QMessageBox;
class QTimer;
class ObjectUpdater;


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

class PlotViewer: public DataViewer
{
    Q_OBJECT
    friend class PlotViewerDispatcher;
public:
    PlotViewer(Ui_DataViewer *ui, QWidget * parent = 0);
    ~PlotViewer();
    void sendPlotCmd(QString name);

public slots:
    virtual void open() Q_DECL_OVERRIDE;
    virtual void save() Q_DECL_OVERRIDE;
    virtual void copy() Q_DECL_OVERRIDE;
    void updateAllActivePlots();
    virtual void addRequestedItem(QString name="", bool isBackup=false);
    virtual void snapshot(QString name="", QString snapshotName="") Q_DECL_OVERRIDE;
    virtual bool setCurrentItem(QString name) Q_DECL_OVERRIDE;


protected slots:
    virtual void destroyItem_impl(QString name) Q_DECL_OVERRIDE;
    void resizeTimeout();
    void setupFullScreen();
    virtual void enableActions(bool enable) Q_DECL_OVERRIDE;
    void sendPlotCmd();
    void displaySVG(QByteArray byteArray, QString cmd);
    void setPlotByteArray(QByteArray byteArray, QString cmd);
    void storeCopyByteArray(QByteArray byteArray, QString cmd);
    void assignStoredCopyByteArray();
    void requestAddDataframe(QString name="", bool isBackup=false);
    void restartTimer();


signals:
     void setPlotSettings(QString);
     void removePlotSettings(QString);
     void addDataframeRequested(QString, bool);

protected:
     virtual void addItem_impl(QString name) Q_DECL_OVERRIDE;
     virtual QWidget *makeView(QString name);
     virtual void addNameToFilter(QString name);
     virtual void removeNameFromFilter(QString name);
     virtual void setNameInFilter(QString name);

     void updateActivePlots();
     QSvgWidget *currentSvgWidget();
     void addExtraActions();
     void paintEvent(QPaintEvent * event);
     void resizeEvent(QResizeEvent*);

    bool            pend;
    double          plotAspectRatio;
    QTimer*         resizeTimer;

    QMap <QString, bool> plotIsActive;
    QMap <QString, bool> plotIsUpToDate;
    QMap <QString, QByteArray> plotByteArray;
     QMap <QString, double> plotLastRatio;
     QByteArray copyByteArray;


    QAction *       fullScreenAct;
    bool            fullScreen;
    QSize           fullScreenSize;
    double          fullScreenAspectRatio;

    FullScreenSvgDialog *fullScreenSvgDialog;


};

#endif // PLOTVIEWER_H
