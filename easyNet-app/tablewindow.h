#ifndef TABLEWINDOW_H
#define TABLEWINDOW_H

#include <QMainWindow>


class TableWindow: public QMainWindow
{
    Q_OBJECT

public:
    explicit TableWindow(QWidget *parent = 0);
    ~TableWindow();
};

#endif // TABLEWINDOW_H
