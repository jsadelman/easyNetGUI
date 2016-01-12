#ifndef DATAVIEWER_H
#define DATAVIEWER_H

#include <QDockWidget>

class Ui_DataViewer;


class DataViewer : public QDockWidget
{
    Q_OBJECT
public:
    DataViewer(QWidget * parent = 0);
    ~DataViewer();
    void setUi(Ui_DataViewer *ui_dataViewer);

protected slots:
    virtual void open()=0;
    virtual void save()=0;
    virtual void copy()=0;
    virtual void removeItem(QString name)=0;
    virtual void updateCurrentItem(QString name)=0;
    virtual void enableActions(bool enable);

    Ui_DataViewer *ui;
};

#endif // DATAVIEWER_H
