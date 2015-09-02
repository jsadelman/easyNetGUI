#ifndef PLOTVIEWER_H
#define PLOTVIEWER_H

#include <QMainWindow>
#include <QMap>
#include <QTimer>

class QSvgWidget;
class QToolBar;
class QAction;
class QLabel;
class QByteArray;

class PlotViewer: public QMainWindow
{
    Q_OBJECT

public:
    PlotViewer(QString easyNetHome, QWidget *parent=0);
    ~PlotViewer();

    QTabWidget*     plotPanel;
//    QSvgWidget*     svgWidget;
//    QVector <QSvgWidget*>     plots;
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
private:
    void createToolBars();
    void createActions();
    void resizeEvent(QResizeEvent*);

    QString         easyNetHome;
    QLabel*         titleLabel;
    QByteArray      byteArray;
//    int             currentPlotIdx;
    int             progressiveTabIdx;
    QMap <QSvgWidget*, QString> plotName;
//    QMap <QSvgWidget*, bool> plotSettingsOn;
    QMap <QSvgWidget*, bool> plotIsActive;
    QTimer*         resizeTimer;
};

#endif // PLOTVIEWER_H
