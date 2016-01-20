#ifndef UI_DATATABSVIEWER_H
#define UI_DATATABSVIEWER_H

#include "ui_dataviewer.h"


class QTabWidget;



class Ui_DataTabsViewer : public Ui_DataViewer
{
    Q_OBJECT
public:
    Ui_DataTabsViewer();
    ~Ui_DataTabsViewer();
    virtual QString currentItem();
    virtual void setCurrentItem(QString name);

public slots:
    virtual void addItem(QString name, QWidget *item) Q_DECL_OVERRIDE;
    virtual void removeItem(QString name) Q_DECL_OVERRIDE;
    virtual void replaceItem(QString name, QWidget *item) Q_DECL_OVERRIDE;

protected:
    virtual void createViewer();
    virtual void displayPrettyName(QString name);

private:
    QTabWidget *tabWidget;
    bool quiet_tab_change;

};

#endif // UI_DATATABSVIEWER_H
