#include "diagramwindow.h"
#include "diagramSceneStackedWidget.h"
#include "diagramscenestackedwidget.h"
#include "diagramscene.h"
#include "diagramview.h"


#include <QtWidgets>
#include <QSignalMapper>
#include <QTransform>
#include <QDebug>



//DiagramWindow::DiagramWindow(diagramSceneStackedWidget *diagramSceneStackedWidget, QWidget *parent)
//    : diagramSceneStackedWidget(diagramSceneStackedWidget), QMainWindow(parent)
DiagramWindow::DiagramWindow(DiagramSceneStackedWidget *diagramSceneStackedWidget, QWidget *parent)
    : diagramSceneStackedWidget(diagramSceneStackedWidget), QMainWindow(parent)
{

    setCentralWidget(diagramSceneStackedWidget);
    createMenus();
    connect(diagramSceneStackedWidget, SIGNAL(initArrangement(DiagramScene*)), this, SLOT(initArrangement(DiagramScene*)));
    connect(diagramSceneStackedWidget, SIGNAL(zoomChanged()), this, SLOT(restoreZoom()));
}

void DiagramWindow::rearrange(bool ignoreEdges)
{

//    if (fitVisibleAct->isChecked())
//        connect(diagramSceneStackedWidget->currentDiagramScene(), SIGNAL(animationFinished()),
//                this, SLOT(toFitVisible()));
#if 0
    diagramSceneStackedWidget->currentCanvas()->layout()->runDirect(true, ignoreEdges);
    diagramSceneStackedWidget->currentDiagramScene()->processLayoutUpdateEvent();
#endif
    diagramSceneStackedWidget->currentDiagramScene()->initShapePlacement();
}

void DiagramWindow::arrange(bool ignoreEdges)
{
#if 0
    diagramSceneStackedWidget->currentCanvas()->layout()->clearFixedList();
#endif
    rearrange(ignoreEdges);

}

void DiagramWindow::initArrangement(DiagramScene *scene)
{
    fitVisibleAct->setChecked(true);
     if (QFileInfo(scene->layoutFile()).exists())
    {
        loadLayout();
    }
    else
    {
        scene->initShapePlacement();
    }
}

void DiagramWindow::sceneScaleChanged(const QString &scale)
{
    QRegExp getScaleRex("^(\\d+)%?");
    if (getScaleRex.indexIn(scale) != -1)
    {
        double newScale = getScaleRex.cap(1).toDouble() / 100.0;
        double oldScale = diagramSceneStackedWidget->currentCanvasView()->transform().m11();
        if (qAbs(newScale - oldScale) >= 0.01)
        {
            DiagramView *view = diagramSceneStackedWidget->currentDiagramView();
            QMatrix oldMatrix = view->matrix();
            view->resetMatrix();
            view->translate(oldMatrix.dx(), oldMatrix.dy());
            view->scale(newScale, newScale);
            emit view->zoomChanged();
        }
    }
}

void DiagramWindow::fitVisible(bool on)
{
    if (on)
    {
        connect(diagramSceneStackedWidget->currentDiagramView(), SIGNAL(canvasViewResized()),
                diagramSceneStackedWidget->currentDiagramView(), SLOT(fitVisible()));
        diagramSceneStackedWidget->currentDiagramView()->fitVisible();
    }
    else
        disconnect(diagramSceneStackedWidget->currentDiagramView(), SIGNAL(canvasViewResized()),
                diagramSceneStackedWidget->currentDiagramView(), SLOT(fitVisible()));
}

void DiagramWindow::restoreZoom()
{
    QGraphicsView *view = diagramSceneStackedWidget->currentCanvasView();
    double m11 = view->transform().m11();
    QString zoomText;
    zoomText.setNum((int)(100 * m11)).append("\%");
    sceneScaleCombo->setCurrentText(zoomText);
}

void DiagramWindow::restoreProperties()
{
    idealEdgeLengthModifierSpinBox->setValue(diagramSceneStackedWidget->currentCanvas()->property("idealEdgeLengthModifier").toDouble());
    shapeNonOverlapPaddingSpinBox->setValue(diagramSceneStackedWidget->currentCanvas()->property("shapeNonOverlapPadding").toInt());
    flowSeparationModifierSpinBox->setValue(diagramSceneStackedWidget->currentCanvas()->property("flowSeparationModifier").toDouble());
}

void DiagramWindow::restore()
{
    fitVisibleAct->setChecked(false);
    restoreZoom();
}

void DiagramWindow::deleteSelection()
{
    diagramSceneStackedWidget->currentCanvas()->deleteSelection();
}

void DiagramWindow::alignSelection(DiagramScene::Alignment alignType)
{
//    if (!diagramSceneStackedWidget->currentCanvas()->selectedItems().isEmpty())
#if 0
    if (diagramSceneStackedWidget->currentDiagramScene()->validForAlignment(
                diagramSceneStackedWidget->currentCanvas()->selectedItems()))
#endif
    {
        diagramSceneStackedWidget->currentCanvas()->alignSelection(alignType);
    }
}


void DiagramWindow::toFitVisible(DiagramScene *scene)
{

    if (diagramSceneStackedWidget->currentDiagramView()->canvas() == scene)
    {
        qDebug() << Q_FUNC_INFO;
        diagramSceneStackedWidget->currentDiagramView()->fitVisible();
    }
//    disconnect(scene, SIGNAL(animationFinished()),
//               this, SLOT(toFitVisible(DiagramScene *)));
}

void DiagramWindow::setZoom()
{
    fitVisibleAct->setChecked(false);
    sceneScaleChanged(sceneScaleCombo->currentText());
}

void DiagramWindow::loadLayout()
{
    fitVisibleAct->setChecked(false);
    diagramSceneStackedWidget->currentDiagramView()->loadLayout();
    fitVisibleAct->setChecked(true);
}

void DiagramWindow::saveLayout()
{
    diagramSceneStackedWidget->currentDiagramView()->saveLayout();
}

void DiagramWindow::createMenus()
{
//    QToolBar *layoutToolBar = addToolBar("Auto layout");

//    arrangeActButton = new QPushButton(tr("Arrange"));
//    connect(arrangeActButton, SIGNAL(clicked()), this, SLOT(arrange()));

//    QAction *initArrangementAct = new QAction(tr("Init"), this);
//    connect(initArrangementAct, SIGNAL(triggered()), this, SLOT(initArrangement()));
//    layoutToolBar->addAction(initArrangementAct);

//    fitVisibleButton = new QRadioButton(tr("Fit Visible"));
//    connect(fitVisibleButton, SIGNAL(toggled(bool)), this, SLOT(fitVisible(bool)));
//    fitVisibleButton->setChecked(false);


#if 0
    QGroupBox * layoutModeBox = new QGroupBox("Layout modes");
    QRadioButton *organicButton = new QRadioButton(tr("Organic"));
    QRadioButton *flowButton = new QRadioButton(tr("Flow"));
    QRadioButton *layeredButton = new QRadioButton(tr("Layered"));


    QVBoxLayout *layoutModeVboxLayout = new QVBoxLayout;
    layoutModeVboxLayout->addWidget(organicButton);
    layoutModeVboxLayout->addWidget(flowButton);
    layoutModeVboxLayout->addWidget(layeredButton);
//    layoutModeVboxLayout->addStretch();
    layoutModeBox->setLayout(layoutModeVboxLayout);

    connect(organicButton, &QRadioButton::toggled, [=](){
        diagramSceneStackedWidget->currentCanvas()->setProperty("layoutMode", Canvas::OrganicLayout);
    });
    connect(flowButton, &QRadioButton::toggled, [=](){
        diagramSceneStackedWidget->currentCanvas()->setProperty("layoutMode", Canvas::FlowLayout);
    });
    connect(layeredButton, &QRadioButton::toggled, [=](){
        diagramSceneStackedWidget->currentCanvas()->setProperty("layoutMode", Canvas::LayeredLayout);
    });

    organicButton->setChecked(true);
#endif

//    QDockWidget *layoutDock = new QDockWidget("Layouts");
//    //     layoutDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
//    layoutDock->setWidget(layoutModeBox);

//    addDockWidget(Qt::LeftDockWidgetArea, layoutDock);

//    loadModelAct = new QAction(QIcon(":/images/layers-2x.png"), tr("&Load model"), this);
    loadModelAct = new QAction(QIcon(":/images/modelChooserIcon.png"), tr("&Load model from selector"), this);
    loadModelAct->setShortcuts(QKeySequence::Open);
    loadModelAct->setStatusTip(tr("Load a previously specified model"));
    connect(loadModelAct, SIGNAL(triggered()), this, SIGNAL(loadModelSignal()));

    loadModelFileAct = new QAction(QIcon(":/images/open.png"), tr("&Load model from file dialog"), this);
    loadModelFileAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_J));
    loadModelFileAct->setStatusTip(tr("Load a previously specified model from file dialog"));
    connect(loadModelFileAct, SIGNAL(triggered()), this, SIGNAL(loadModelFileSignal()));


    modelSettingsAct = new QAction(QIcon(":/images/list-4x.png"),tr("Model Settings"), this);
    modelSettingsAct->setStatusTip(tr("Display model settings"));
    connect(modelSettingsAct,SIGNAL(triggered()), this,SIGNAL(showModelSettingsSignal()));

    parameterSettingsAct = new QAction(QIcon(":/images/parameters.png"),tr("Model Parameters"), this);
    parameterSettingsAct->setStatusTip(tr("Display parameter settings"));
    connect(parameterSettingsAct,SIGNAL(triggered()), this,SIGNAL(showParameterSettingsSignal()));

    disableAllObserversAct = new QAction(QIcon(":/images/clean_brush-128.png"),tr("Stop observing"), this);
    connect(disableAllObserversAct,SIGNAL(triggered()), this, SLOT(disableAllObservers()));

    fitVisibleAct = new QAction(QIcon(":/images/resize.png"),tr("Fit to window"), this);
    fitVisibleAct->setStatusTip(tr("Resize diagram to fit window"));
    fitVisibleAct->setCheckable(true);
    connect(fitVisibleAct,SIGNAL(toggled(bool)), this,SLOT(fitVisible(bool)));
    fitVisibleAct->setChecked(false);

    QAction* arrangeAct = new QAction(QIcon(":/images/magic_wand.png"),tr("Auto-arrange"), this);
    arrangeAct->setStatusTip(tr("Auto-arrange network layout"));
    connect(arrangeAct, SIGNAL(triggered()), this, SLOT(arrange()));

    QAction* loadLayoutAct = new QAction(QIcon(":/images/layers-2x.png"), tr("Load layout"), this);
    loadLayoutAct->setStatusTip(tr("Reload layout"));
    connect(loadLayoutAct, SIGNAL(triggered()), this, SLOT(loadLayout()));

    QAction* saveLayoutAct = new QAction(QIcon(":/images/save.png"), tr("Save layout"), this);
    saveLayoutAct->setStatusTip(tr("Save layout"));
    connect(saveLayoutAct, SIGNAL(triggered()), this, SLOT(saveLayout()));

    // ALIGNMENT

    QAction* vertAlign = new QAction(QIcon(":/images/verticalAlign.png"),tr("Vertical align"), this);
    QAction* horizAlign = new QAction(QIcon(":/images/horizontalAlign.png"),tr("Horizontal align"), this);
    vertAlign->setStatusTip(tr("Vertically align the middle of selected layers"));
    horizAlign->setStatusTip(tr("Horizontally align the middle of selected layers"));
    connect(vertAlign,SIGNAL(triggered()),this,SLOT(vertAlignSlot()));
    connect(horizAlign,SIGNAL(triggered()),this,SLOT(horizAlignSlot()));

    // ZOOM
    sceneScaleCombo = new QComboBox;
    QStringList scales;
    scales <<  tr("25%") << tr("33%") <<  tr("50%") << tr("75%") << tr("100%");
    sceneScaleCombo->addItems(scales);
    sceneScaleCombo->setCurrentIndex(4);

    QRegExp zoomRex("\\d{1,3}%?");
    QValidator *zoomValidator = new QRegExpValidator(zoomRex, sceneScaleCombo);
    QLineEdit *zoomEdit = new QLineEdit(sceneScaleCombo);
    zoomEdit->setValidator(zoomValidator);
    zoomEdit->setMinimumWidth(QFontMetrics(zoomEdit->font()).width("100%"));
    sceneScaleCombo->setLineEdit(zoomEdit);
    sceneScaleCombo->setInsertPolicy(QComboBox::NoInsert);

     connect(sceneScaleCombo, SIGNAL(currentIndexChanged(int)),
             this, SLOT(setZoom()));
     connect(sceneScaleCombo->lineEdit(), SIGNAL(returnPressed()),
             this, SLOT(setZoom()));

//     for (int i = 0; i < diagramSceneStackedWidget->count(); ++i)
//         connect(diagramSceneStackedWidget->diagramViewAt(i), SIGNAL(zoomChanged()),
//                 this, SLOT(restoreZoom()));

#if 0
    // PROPERTIES
    QLabel *idealEdgeLengthModifierLabel = new QLabel("idealEdgeLengthModifier");
    idealEdgeLengthModifierSpinBox = new QDoubleSpinBox();

    QLabel *shapeNonOverlapPaddingLabel = new QLabel("shapeNonOverlapPadding");
    shapeNonOverlapPaddingSpinBox = new QSpinBox();

    QLabel *flowSeparationModifierLabel = new QLabel("flowSeparationModifier");
    flowSeparationModifierSpinBox = new QDoubleSpinBox();

    QGridLayout *propertiesLayout = new QGridLayout;
    propertiesLayout->addWidget(idealEdgeLengthModifierLabel, 0, 0);
    propertiesLayout->addWidget(idealEdgeLengthModifierSpinBox, 0, 1);
    propertiesLayout->addWidget(shapeNonOverlapPaddingLabel, 1, 0);
    propertiesLayout->addWidget(shapeNonOverlapPaddingSpinBox, 1, 1);
    propertiesLayout->addWidget(flowSeparationModifierLabel, 2, 0);
    propertiesLayout->addWidget(flowSeparationModifierSpinBox, 2, 1);

    QGroupBox * propertiesBox = new QGroupBox("Properties");
    propertiesBox->setLayout(propertiesLayout);

    connect(idealEdgeLengthModifierSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value){
        diagramSceneStackedWidget->currentCanvas()->setProperty("idealEdgeLengthModifier", value);
    });
    connect(shapeNonOverlapPaddingSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int value){
        diagramSceneStackedWidget->currentCanvas()->setProperty("shapeNonOverlapPadding", value);
    });
    connect(flowSeparationModifierSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value){
        diagramSceneStackedWidget->currentCanvas()->setProperty("flowSeparationModifier", value);
    });
    connect(diagramSceneStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(restoreProperties()));

    restoreProperties();
#endif

    diagramTopToolBar = new QToolBar("Details");
    addToolBar(Qt::TopToolBarArea,diagramTopToolBar);
    diagramTopToolBar->setMovable(false);
    diagramTopToolBar->addAction(loadModelAct);
    diagramTopToolBar->addAction(loadModelFileAct);
    diagramTopToolBar->addSeparator();
    diagramTopToolBar->addAction(modelSettingsAct);
    diagramTopToolBar->addAction(parameterSettingsAct);
    diagramTopToolBar->addAction(disableAllObserversAct);
    diagramTopToolBar->addSeparator();

    modelNameLabel = new QLabel();
    modelNameLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    modelNameLabel->setStyleSheet("QLabel {"
                            "qproperty-alignment: AlignCenter;"
                            "padding-right: 3px;"
                            "padding-left: 3px;"
                            "font-weight: bold;"
                            "background-color: white;"
                            "}");

    diagramTopToolBar->addWidget(modelNameLabel);
    diagramTopToolBar->addWidget(sceneScaleCombo);
    diagramTopToolBar->addAction(fitVisibleAct);
    diagramTopToolBar->addAction(vertAlign);
    diagramTopToolBar->addAction(horizAlign);
    diagramTopToolBar->addAction(arrangeAct);
    diagramTopToolBar->addSeparator();
    diagramTopToolBar->addAction(loadLayoutAct);
    diagramTopToolBar->addAction(saveLayoutAct);

    connect(diagramSceneStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(restore()));

}

void DiagramWindow::vertAlignSlot()
{
    alignSelection (DiagramScene::Vertical);
}

void DiagramWindow::horizAlignSlot()
{
    alignSelection (DiagramScene::Horizontal);
}

void DiagramWindow::setModelName(QString name)
{
    modelNameLabel->setText(name);
}

void DiagramWindow::disableAllObservers()
{
    diagramSceneStackedWidget->currentDiagramScene()->disableAllObservers();
}

