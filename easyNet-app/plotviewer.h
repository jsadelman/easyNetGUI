#ifndef PLOTVIEWER_H
#define PLOTVIEWER_H

#include <QMainWindow>
#include <QMap>

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
    QSvgWidget*     svgWidget;
    QVector <QSvgWidget*>     plots;
    QToolBar*       fileToolBar;
    QToolBar*       editToolBar;
    QToolBar*       navigationToolBar;
    QAction *       settingsAct;
    QAction *       refreshAct;
    QAction *       openAct;
    QAction *       saveAct;
    QAction *       copyAct;

public slots:
    void updateActivePlots();
signals:
    void sendDrawCmd(QString);
    void showPlotSettings();

private slots:
    void loadSVGFile();
    void loadByteArray(QString name, QByteArray byteArray);
    void save();
    void copySVGToClipboard();
    void addPlot(QString name);
private:
    void createToolBars();
    void createActions();

    QString         easyNetHome;
    QLabel*         titleLabel;
    QByteArray      byteArray;
    int             currentPlotIdx;
    int             numPlots;
    QMap <int, QString> plotMap;

};

#endif // PLOTVIEWER_H
