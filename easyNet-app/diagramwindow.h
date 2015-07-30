#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#include <QMainWindow>

class DiagramSceneTabWidget;

class DiagramWindow
        : public QMainWindow
{
    Q_OBJECT
public:
    DiagramWindow(DiagramSceneTabWidget *diagramSceneTabWidget, QWidget *parent = 0);

public slots:
    void arrange();
    void rearrange();

private:
    void createMenus();
    DiagramSceneTabWidget *diagramSceneTabWidget;

};

#endif // DIAGRAMWINDOW_H
