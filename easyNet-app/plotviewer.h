#ifndef PLOTVIEWER_H
#define PLOTVIEWER_H

#include <QMainWindow>
#include <QMap>
#include <QTimer>
#include <QDialog>

class QSvgWidget;
class QToolBar;
class QAction;
class QLabel;
class QByteArray;
class QScrollArea;


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



class PlotViewer: public QMainWindow
{
    Q_OBJECT

public:
    PlotViewer(QString easyNetHome, QWidget *parent=0);
    ~PlotViewer();

    QTabWidget*     plotPanel;
    QSvgWidget *currentSvgWidget();


public slots:
    void updateActivePlots();
signals:
    void sendDrawCmd(QString);
    void showPlotSettings();
    void setPlot(QString);
    void resized(QSize);
    void hidePlotSettings();



private slots:
    void loadSVGFile();
    void loadByteArray(QString name, QByteArray byteArray);
    void save();
    void copySVGToClipboard();
    void addPlot(QString name);
    void resizeTimeout();
    void snapshot();
    void currentTabChanged(int index);
    void freeze(QSvgWidget *svg = nullptr);
    void renamePlot();
    void deletePlot();
    void makeSnapshot(QString name);
    void setupFullScreen();


private:
    void createToolBars();
    void createActions();
    void paintEvent(QPaintEvent * event);
    void resizeEvent(QResizeEvent*);
    void setPlotActive(bool isActive, QSvgWidget *svg = nullptr);

    FullScreenSvgDialog *fullScreenSvgDialog;

    QToolBar*       fileToolBar;
    QToolBar*       editToolBar;
    QToolBar*       navigationToolBar;
    QAction *       settingsAct;
    QAction *       refreshAct;
    QAction *       snapshotAct;
    QAction *       renameAct;
    QAction *       openAct;
    QAction *       saveAct;
    QAction *       copyAct;
    QAction *       deleteAct;
    QAction *       fullScreenAct;

    QString         easyNetHome;
    QLabel*         titleLabel;
    int             progressiveTabIdx;
    QMap <QSvgWidget*, QString> plotName;
    QMap <QSvgWidget*, bool> plotIsActive;
    QMap <QSvgWidget*, QByteArray> byteArray;
    QMap <QSvgWidget*, bool> plotIsUpToDate;
    QTimer*         resizeTimer;
    bool            pend;
    bool            fullScreen;
    QSize           fullScreenSize;
};

#endif // PLOTVIEWER_H
