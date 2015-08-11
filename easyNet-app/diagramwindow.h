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
    void rearrange();
    void arrange();
    void initArrangement();

private slots:
    void sceneScaleChanged(const QString &scale);
    void fitVisible(bool on);
    void disableFitVisible() {fitVisible(false);}
    void enableFitVisible()  {fitVisible(true);}
    void restoreZoom();
    void restoreProperties();
    void restore();
    void deleteSelection();
    void alignSelection(int alignType);
    void toFitVisible();
    void setZoom();
    void loadLayout();
    void saveLayout();
private:
    void createMenus();
    DiagramSceneTabWidget *diagramSceneTabWidget;
    QComboBox *sceneScaleCombo;
    QDoubleSpinBox *idealEdgeLengthModifierSpinBox;
    QSpinBox *shapeNonOverlapPaddingSpinBox;
    QDoubleSpinBox *flowSeparationModifierSpinBox;
    QRadioButton *fitVisibleButton;

};

#endif // DIAGRAMWINDOW_H
