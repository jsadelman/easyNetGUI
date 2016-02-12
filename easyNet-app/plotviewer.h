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
    QString plotType(QString name);

public slots:
    virtual void open() Q_DECL_OVERRIDE;
    virtual void save() Q_DECL_OVERRIDE;
    virtual void copy() Q_DECL_OVERRIDE;
    void updateAllActivePlots();


protected slots:
    virtual void destroyItem_impl(QString name) Q_DECL_OVERRIDE;
    void resizeTimeout();
    void dfSourceModified(QString df);
    void generatePrettyName(QString plotName, QString type, QString subtype, QDomDocument* domDoc);
    void setupFullScreen();
    void addSourceDataframes(QStringList newDataframes=QStringList());
    virtual void enableActions(bool enable) Q_DECL_OVERRIDE;
    virtual void updateCurrentItem(QString name) Q_DECL_OVERRIDE;
    void updatePlot(QString name, QByteArray byteArray);


signals:
     void resized(QSize);
     void sendDrawCmd(QString);
     void showPlotSettings();
     void createNewRPlot(QString, QString, QMap<QString, QString>, int, QList<QSharedPointer<QDomDocument> >);
     void setPlotSettings(QString);
     void removePlotSettings(QString);

protected:
     virtual QWidget *makeView(QString name);
     virtual void addNameToFilter(QString name);
     virtual void removeNameFromFilter(QString name);
     virtual void setNameInFilter(QString name);

     void updateActivePlots();
     QSvgWidget *currentSvgWidget();
     void addExtraActions();
     QString cloneRPlot(QString name, QString newName=QString());
     void paintEvent(QPaintEvent * event);
     void resizeEvent(QResizeEvent*);

    bool            pend;
    QTimer*         resizeTimer;
    ObjectCacheFilter *sourceDataframeFilter;
    ObjectCacheFilter *plotDescriptionFilter;
    ObjectUpdater     *plotDescriptionUpdater;
    QMap <QString, bool> plotIsActive;
    QMap <QString, bool> plotIsUpToDate;
    QMap <QString, bool> plotSourceModified;
    QMap <QString, QByteArray> plotByteArray;


    QAction *       settingsAct;
    QAction *       fullScreenAct;
    bool            fullScreen;
    QSize           fullScreenSize;

    FullScreenSvgDialog *fullScreenSvgDialog;


};

#endif // PLOTVIEWER_H
