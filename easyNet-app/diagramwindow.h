#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#include <QMainWindow>
#include <QTabBar>
#include "diagramscene.h"
class DiagramSceneTabWidget;
class DiagramSceneStackedWidget;
class DiagramScene;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QRadioButton;
class QPushButton;
class QToolButton;
class QVBoxLayout;
class QLabel;

class DiagramWindow
        : public QMainWindow
{
    Q_OBJECT
public:
//    DiagramWindow(DiagramSceneTabWidget *diagramSceneTabWidget, QWidget *parent = 0);
    DiagramWindow(DiagramSceneStackedWidget *diagramSceneStackedWidget, QWidget *parent = 0);
    QAction* parameterSettingsAct;


public slots:
    void rearrange(bool ignoreEdges = false);
    void arrange(bool ignoreEdges = false);
    void initArrangement(DiagramScene *scene);
    void toFitVisible(DiagramScene *scene);


signals:
    void loadModelSignal();
    void loadModelFileSignal();
    void showModelSettingsSignal();
    void showParameterSettingsSignal();
private slots:
    void sceneScaleChanged(const QString &scale);
    void fitVisible(bool on);
    void disableFitVisible() {fitVisible(false);}
    void enableFitVisible()  {fitVisible(true);}
    void restoreZoom();
    void restoreProperties();
    void restore();
    void deleteSelection();
    void alignSelection(DiagramScene::Alignment alignType);
    void setZoom();
    void loadLayout();
    void saveLayout();
    void vertAlignSlot();
    void horizAlignSlot();
    void setModelName(QString name);
    void disableAllObservers();
private:
    void createMenus();
//    DiagramSceneTabWidget *diagramSceneTabWidget;
    DiagramSceneStackedWidget *diagramSceneStackedWidget;
    QComboBox *sceneScaleCombo;
    QDoubleSpinBox *idealEdgeLengthModifierSpinBox;
    QSpinBox *shapeNonOverlapPaddingSpinBox;
    QDoubleSpinBox *flowSeparationModifierSpinBox;
//    QRadioButton *fitVisibleButton;
    QAction* loadModelAct;
    QAction* loadModelFileAct;
    QAction* modelSettingsAct;
    QAction* disableAllObserversAct;
    QAction* fitVisibleAct;
    QPushButton *loadLayoutButton;
    QPushButton *saveLayoutButton;
    QPushButton *deleteAlignmentButton;
    QPushButton *arrangeActButton;
    QToolBar *diagramTopToolBar;
    QLabel *modelNameLabel;
    bool hidden;

};


#endif // DIAGRAMWINDOW_H
