#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#include <QMainWindow>
#include <QTabBar>

class DiagramSceneTabWidget;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QRadioButton;
class QPushButton;
class QToolButton;
class QVBoxLayout;

class DiagramWindow
        : public QMainWindow
{
    Q_OBJECT
public:
    DiagramWindow(DiagramSceneTabWidget *diagramSceneTabWidget, QWidget *parent = 0);

public slots:
    void rearrange(bool ignoreEdges = false);
    void arrange(bool ignoreEdges = false);
    void initArrangement();

    void ToggleControlsDock();
signals:
    showModelSettingsSignal();
    showParameterSettingsSignal();
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
    void vertAlignSlot();
    void horizAlignSlot();
private:
    void createMenus();
    DiagramSceneTabWidget *diagramSceneTabWidget;
    QComboBox *sceneScaleCombo;
    QDoubleSpinBox *idealEdgeLengthModifierSpinBox;
    QSpinBox *shapeNonOverlapPaddingSpinBox;
    QDoubleSpinBox *flowSeparationModifierSpinBox;
//    QRadioButton *fitVisibleButton;
    QAction* modelSettingsAct;
    QAction* parameterSettingsAct;
    QAction* fitVisibleAct;
    QPushButton *loadLayoutButton;
    QPushButton *saveLayoutButton;
    QPushButton *deleteAlignmentButton;
    QPushButton *arrangeActButton;
    QDockWidget *controlsDock;
    QToolBar *diagramToolBar;
    QToolBar *diagramTopToolBar;
    bool hidden;

};


#endif // DIAGRAMWINDOW_H
