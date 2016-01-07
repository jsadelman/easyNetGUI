#ifndef UI_DATAVIEWER_H
#define UI_DATAVIEWER_H

#include <QMainWindow>
#include <QMap>

class DataViewer;
class QSignalMapper;
class QActionGroup;

class Ui_DataViewer: public QMainWindow
{
    Q_OBJECT
public:
    Ui_DataViewer(QWidget * parent = 0);
    ~Ui_DataViewer();
    virtual void setupUi(DataViewer *dataViewer)=0;
    virtual QString currentItem()=0;
    virtual void setCurrentItem(QString name)=0;

    QAction *openAct;
    QAction *saveAct;
    QAction *copyAct;
//    QAction *infoAct;

    QList<QAction *> setDispatchModeOverrideActs;
    QSignalMapper *setDispatchModeOverrideMapper;
    QActionGroup *setDispatchModeOverrideActGroup;
    QAction *setDispatchModeAutoAct;


    QMap<int, QString> dispatchModeName;
    QMap<int, QString> dispatchModeIconName;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *dispatchToolBar;
//    QToolBar *infoToolBar;


public slots:
    virtual void addItem(QString name, QWidget *item)=0;
    virtual void removeItem(QString name)=0;

signals:
    void deleteItemRequested(QString name);
    void currentItemChanged(QString name);

protected:
    virtual void createActions();
    virtual void createToolBars();
    QMap<QString, QWidget *> itemMap;


};

#endif // UI_DATAVIEWER_H
