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
    virtual QString currentItemName();
    virtual QWidget *currentView() Q_DECL_OVERRIDE;
    void setTabsClosable(bool closeable) {tabsClosable = closeable;}


public slots:
    virtual void addView(QString name, QWidget *view) Q_DECL_OVERRIDE;
    virtual QWidget *takeView(QString name) Q_DECL_OVERRIDE;
    virtual void setCurrentItem(QString name);

protected:
    virtual void createViewer();
    virtual void displayPrettyName(QString name);

private:
    QTabWidget *tabWidget;
    bool tabsClosable;
    bool quiet_tab_change;

};

#endif // UI_DATATABSVIEWER_H
