#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#include <QMainWindow>

class DiagramSceneTabWidget;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;

class DiagramWindow
        : public QMainWindow
{
    Q_OBJECT
public:
    DiagramWindow(DiagramSceneTabWidget *diagramSceneTabWidget, QWidget *parent = 0);

public slots:
    void arrange();
    void rearrange();

private slots:
    void sceneScaleChanged(const QString &scale);

    void recoverZoom();
    void recoverProperties();
    void deleteSelection();
    void alignSelection(int alignType);
private:
    void createMenus();
    DiagramSceneTabWidget *diagramSceneTabWidget;
    QComboBox *sceneScaleCombo;
    QDoubleSpinBox *idealEdgeLengthModifierSpinBox;
    QSpinBox *shapeNonOverlapPaddingSpinBox;
    QDoubleSpinBox *flowSeparationModifierSpinBox;

};

#endif // DIAGRAMWINDOW_H
