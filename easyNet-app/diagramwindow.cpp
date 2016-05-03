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
    connect(diagramSceneStackedWidget, SIGNAL(initArrangement()), this, SLOT(initArrangement()));
    connect(diagramSceneStackedWidget, SIGNAL(zoomChanged()), this, SLOT(restoreZoom()));
}


void DiagramWindow::rearrange(bool ignoreEdges)
{

    if (fitVisibleAct->isChecked())
        connect(diagramSceneStackedWidget->currentDiagramScene(), SIGNAL(animationFinished()),
                this, SLOT(toFitVisible()));
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

void DiagramWindow::initArrangement()
{

    fitVisibleAct->setChecked(true);
    connect(diagramSceneStackedWidget->diagramSceneAt(0), SIGNAL(animationFinished()),
            this, SLOT(toFitVisible()));

    if (QFileInfo(diagramSceneStackedWidget->diagramSceneAt(0)->layoutFile()).exists())
    {
        loadLayout();
    }
    else
    {
        diagramSceneStackedWidget->diagramSceneAt(0)->initShapePlacement();
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


void DiagramWindow::toFitVisible()
{
    diagramSceneStackedWidget->currentDiagramView()->fitVisible();
        disconnect(diagramSceneStackedWidget->currentCanvas(), SIGNAL(animationFinished()),
                   this, SLOT(toFitVisible()));
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


    modelSettingsAct = new QAction(QIcon(":/images/list-4x.png"),tr("Model Load-Time Settings"), this);
    modelSettingsAct->setStatusTip(tr("Display model settings"));
    connect(modelSettingsAct,SIGNAL(triggered()), this,SIGNAL(showModelSettingsSignal()));

    parameterSettingsAct = new QAction(QIcon(":/images/parameters.png"),tr("Model Parameters"), this);
    parameterSettingsAct->setStatusTip(tr("Display parameter settings"));
    connect(parameterSettingsAct,SIGNAL(triggered()), this,SIGNAL(showParameterSettingsSignal()));

    fitVisibleAct = new QAction(QIcon(":/images/resize.png"),tr("Fit to window"), this);
    fitVisibleAct->setStatusTip(tr("Resize diagram to fit window"));
    fitVisibleAct->setCheckable(true);
    connect(fitVisibleAct,SIGNAL(toggled(bool)), this,SLOT(fitVisible(bool)));
    fitVisibleAct->setChecked(false);

    QAction* arrangeAct = new QAction(QIcon(":/images/magic_wand.png"),tr("Auto-arrange"), this);
    arrangeAct->setStatusTip(tr("Auto-arrange network layout"));
    connect(arrangeAct, SIGNAL(triggered()), this, SLOT(arrange()));

    QAction* loadLayoutAct = new QAction(QIcon(":/images/open.png"), tr("Load layout"), this);
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


//    QGroupBox * alignmentBox = new QGroupBox("Alignment");
//    QPushButton *vertLeftButton = new QPushButton("Left");
//    QToolButton *vertCentreButton = new QToolButton("Centre"));
//    QPushButton *vertRightButton = new QPushButton("Right");
//    QPushButton *horiTopButton = new QPushButton("Top");
//    QToolButton *horiCentreButton = new QToolButton("Middle");
//    QPushButton *horiBottomButton = new QPushButton("Bottom");

//    QGridLayout *alignmentLayout = new QGridLayout;
//    alignmentLayout->addWidget(vertLeftButton, 0, 0);
//    alignmentLayout->addWidget(vertCentreButton, 1, 0);
//    alignmentLayout->addWidget(vertRightButton, 2, 0);
//    alignmentLayout->addWidget(horiTopButton, 3, 0);
//    alignmentLayout->addWidget(horiCentreButton, 4, 0);
//    alignmentLayout->addWidget(horiBottomButton, 5, 0);
//    alignmentLayout->addStretch();
//    alignmentBox->setLayout(alignmentLayout);

//    QSignalMapper *alignSignalMapper = new QSignalMapper(this);
////    alignSignalMapper->setMapping(vertLeftButton, (int) dunnart::ALIGN_LEFT);
//    alignSignalMapper->setMapping(vertCentreButton, (int) dunnart::ALIGN_CENTER);
////    alignSignalMapper->setMapping(vertRightButton, (int) dunnart::ALIGN_RIGHT);
////    alignSignalMapper->setMapping(horiTopButton, (int) dunnart::ALIGN_TOP);
//    alignSignalMapper->setMapping(horiCentreButton, (int) dunnart::ALIGN_MIDDLE);
////    alignSignalMapper->setMapping(horiBottomButton, (int) dunnart::ALIGN_BOTTOM);

////    connect(vertLeftButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));
//    connect(vertCentreButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));
////    connect(vertRightButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));
////    connect(horiTopButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));
//    connect(horiCentreButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));
////    connect(horiBottomButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));

//    connect(alignSignalMapper, SIGNAL(mapped(int)),
//            this, SLOT(alignSelection(int)));

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
    diagramTopToolBar->addAction(modelSettingsAct);
    diagramTopToolBar->addAction(parameterSettingsAct);
    diagramTopToolBar->addSeparator();

    diagramToolBar = new QToolBar("Layout");
    addToolBar(Qt::LeftToolBarArea,diagramToolBar);
    diagramToolBar->setMovable(false);
    QToolButton* hideButton = new QToolButton();
    hideButton->setArrowType(Qt::LeftArrow);
    diagramToolBar->addWidget(hideButton);
    diagramToolBar->addSeparator();

    //    diagramToolBar->addWidget(fitVisibleButton);
//    sceneScaleCombo->setMinimumWidth();
//    sceneScaleCombo->setMaximumWidth(60);
//    sceneScaleCombo->setMinimumHeight(40);
//    sceneScaleCombo->setMaximumHeight(40);
    diagramToolBar->addWidget(sceneScaleCombo);
    diagramToolBar->addAction(fitVisibleAct);
//    diagramToolBar->addWidget(alignmentBox);
    diagramToolBar->addAction(vertAlign);
    diagramToolBar->addAction(horizAlign);
//    diagramToolBar->addWidget(vertCentreButton);
//    diagramToolBar->addWidget(horiCentreButton);
//    diagramSceneStackedWidget->addEditToolBarActions(diagramToolBar);
    diagramToolBar->addAction(arrangeAct);
//    diagramToolBar->addWidget(arrangeActButton);
//    diagramToolBar->addWidget(deleteAlignmentButton);
//    diagramToolBar->addWidget(propertiesBox);
    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    diagramToolBar->addWidget(empty);
    diagramToolBar->addSeparator();
    diagramToolBar->addAction(loadLayoutAct);
    diagramToolBar->addAction(saveLayoutAct);

    // to the right of the diagramToolBar is a narrow qdockwidget containing
    // the toolbutton that toggles whether the diagramToolBar is visible
    QToolButton* showButton = new QToolButton();
    showButton->setArrowType(Qt::RightArrow);
    controlsDock = new QDockWidget("");
//    controlsDock->setWidget(controlsWidget);
    controlsDock->setTitleBarWidget(showButton);
    int defaultWidth = 15;
    controlsDock->setMinimumWidth(defaultWidth);
    controlsDock->setMaximumWidth(defaultWidth);
    connect(hideButton, SIGNAL(clicked()), this, SLOT(ToggleControlsDock()));
    connect(showButton, SIGNAL(clicked()), this, SLOT(ToggleControlsDock()));
    addDockWidget(Qt::LeftDockWidgetArea, controlsDock);

    connect(diagramSceneStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(restore()));

    hidden=false;
    controlsDock->hide();
//    ToggleControlsDock();
    diagramToolBar->toggleViewAction()->setChecked(false);
    diagramToolBar->toggleViewAction()->trigger();
}

void DiagramWindow::ToggleControlsDock()
{
    if (hidden)
        controlsDock->hide();
    else
        controlsDock->show();
    hidden = !hidden;
    diagramToolBar->toggleViewAction()->trigger();
}

void DiagramWindow::vertAlignSlot()
{
    alignSelection (DiagramScene::Vertical);
}

void DiagramWindow::horizAlignSlot()
{
    alignSelection (DiagramScene::Horizontal);
}

