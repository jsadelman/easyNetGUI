#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#include <QMainWindow>

class DiagramSceneTabWidget;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QRadioButton;

class DiagramWindow
        : public QMainWindow
{
    Q_OBJECT
public:
    DiagramWindow(DiagramSceneTabWidget *diagramSceneTabWidget, QWidget *parent = 0);

public slots:
    void arrange();
    void rearrange();
    void arrangeNonOverlap();
    void initArrangement();

private slots:
    void sceneScaleChanged(const QString &scale);
    void fitVisible(bool on);
    void restoreZoom();
    void restoreProperties();
    void restore();
    void deleteSelection();
    void alignSelection(int alignType);
    void connectOnceToFitVisible();
    void toFitVisible();
private:
    void createMenus();
    DiagramSceneTabWidget *diagramSceneTabWidget;
    QComboBox *sceneScaleCombo;
    QDoubleSpinBox *idealEdgeLengthModifierSpinBox;
    QSpinBox *shapeNonOverlapPaddingSpinBox;
    QDoubleSpinBox *flowSeparationModifierSpinBox;
    QRadioButton *fitVisibleButton;
    bool disconnectCanvasChanged;

};

#endif // DIAGRAMWINDOW_H
