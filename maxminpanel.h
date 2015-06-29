#ifndef MAXMINPANEL_H
#define MAXMINPANEL_H

#include <QDockWidget>

class MaxMinPanel : public QDockWidget
{
    Q_OBJECT

public:
    MaxMinPanel(QString title, QWidget *parent = 0);
    ~MaxMinPanel();

    bool panelMinimised;

private slots:

    void minimiseCodePanel();

private:

    int userHeight;

};

#endif // MAXMINPANEL_H
