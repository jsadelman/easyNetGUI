#ifndef PLOTVIEWER_H
#define PLOTVIEWER_H

#include <QMainWindow>

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

    QSvgWidget*     svgWidget;
    QToolBar*       fileToolBar;
    QToolBar*       editToolBar;
    QToolBar*       navigationToolBar;
    QAction *       refreshAct;
    QAction *       newAct;
    QAction *       openAct;
    QAction *       saveAct;
    QAction *       copyAct;

signals:
    void sendDrawCmd();

private slots:
    void loadSVGFile();
    void load(QString name, QByteArray byteArray);
    void save();
    void copySVGToClipboard();
private:
    void createToolBars();
    void createActions();

    QString         easyNetHome;
    QLabel*         titleLabel;
    QByteArray      byteArray;

};

#endif // PLOTVIEWER_H
