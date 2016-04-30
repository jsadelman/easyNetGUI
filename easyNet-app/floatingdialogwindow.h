#ifndef FLOATINGDIALOGWINDOW_H
#define FLOATINGDIALOGWINDOW_H

#include <QMainWindow>
#include <QDialog>
class FloatingDialogWindow : public QDialog//MainWindow
{
    Q_OBJECT
public:
    FloatingDialogWindow(QWidget *parent);
    ~FloatingDialogWindow();
    void setCentralWidget(QWidget*);
signals:

public slots:
};

#endif // FLOATINGDIALOGWINDOW_H
