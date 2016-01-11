#ifndef UI_DATATABSVIEWER_H
#define UI_DATATABSVIEWER_H

#include "ui_dataviewer.h"


class QTabWidget;



class Ui_DataTabsViewer : public Ui_DataViewer
{
    Q_OBJECT
public:
    Ui_DataTabsViewer(QWidget * parent = 0);
    ~Ui_DataTabsViewer();
    virtual QString currentItem();
    virtual void setCurrentItem(QString name);

public slots:
    virtual void addItem(QString name, QWidget *item);
    virtual void removeItem(QString name);

protected:
    virtual void createViewer();
    virtual void displayPrettyName(QString name);

private:
    QTabWidget *tabWidget;

};

#endif // UI_DATATABSVIEWER_H
