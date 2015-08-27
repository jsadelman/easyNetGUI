#include "diagramwindow.h"
#include "diagramscenetabwidget.h"
#include "diagramscene.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/canvasview.h"
#include "diagramview.h"


#include <QtWidgets>
#include <QSignalMapper>
#include <QTransform>
#include <QDebug>


using dunnart::Canvas;
using dunnart::CanvasItem;

DiagramWindow::DiagramWindow(DiagramSceneTabWidget *diagramSceneTabWidget, QWidget *parent)
    : diagramSceneTabWidget(diagramSceneTabWidget), QMainWindow(parent)
{
    setCentralWidget(diagramSceneTabWidget);
    createMenus();
    connect(diagramSceneTabWidget, SIGNAL(initArrangement()), this, SLOT(initArrangement()));
}


void DiagramWindow::rearrange()
{
    if (fitVisibleButton->isChecked())
        connect(diagramSceneTabWidget->currentDiagramScene(), SIGNAL(animationFinished()),
                this, SLOT(toFitVisible()));

    diagramSceneTabWidget->currentCanvas()->layout()->runDirect(true);
    diagramSceneTabWidget->currentDiagramScene()->processLayoutUpdateEvent();
}

void DiagramWindow::arrange()
{

//    diagramSceneTabWidget->currentCanvas()->setProperty("preventOverlaps", true);
//    diagramSceneTabWidget->currentCanvas()->setProperty("shapeNonOverlapPadding", 20);
    diagramSceneTabWidget->currentCanvas()->layout()->clearFixedList();
    rearrange();
}

void DiagramWindow::initArrangement()
{

    fitVisibleButton->setChecked(true);

    if (QFileInfo(diagramSceneTabWidget->currentDiagramScene()->layoutFile()).exists())
    {
        connect(diagramSceneTabWidget->currentDiagramScene(), SIGNAL(animationFinished()),
                this, SLOT(toFitVisible()));
        loadLayout();
        diagramSceneTabWidget->currentDiagramScene()->processLayoutUpdateEvent();
    }
    else
    {
        diagramSceneTabWidget->currentDiagramScene()->initShapePlacement();
        arrange();
    }
}

void DiagramWindow::sceneScaleChanged(const QString &scale)
{
    QRegExp getScaleRex("^(\\d+)%?");
    if (getScaleRex.indexIn(scale) != -1)
    {
        double newScale = getScaleRex.cap(1).toDouble() / 100.0;
        double oldScale = diagramSceneTabWidget->currentCanvasView()->transform().m11();
        if (qAbs(newScale - oldScale) >= 0.01)
        {
            DiagramView *view = diagramSceneTabWidget->currentDiagramView();
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
        connect(diagramSceneTabWidget->currentDiagramView(), SIGNAL(canvasViewResized()),
                diagramSceneTabWidget->currentDiagramView(), SLOT(fitVisible()));
        diagramSceneTabWidget->currentDiagramView()->fitVisible();
    }
    else
        disconnect(diagramSceneTabWidget->currentDiagramView(), SIGNAL(canvasViewResized()),
                diagramSceneTabWidget->currentDiagramView(), SLOT(fitVisible()));
}

void DiagramWindow::restoreZoom()
{
    QGraphicsView *view = diagramSceneTabWidget->currentCanvasView();
    double m11 = view->transform().m11();
    QString zoomText;
    zoomText.setNum((int)(100 * m11)).append("\%");
    sceneScaleCombo->setCurrentText(zoomText);
//    sceneScaleCombo->setCurrentIndex(sceneScaleCombo->findText(zoomText));
}

void DiagramWindow::restoreProperties()
{
    idealEdgeLengthModifierSpinBox->setValue(diagramSceneTabWidget->currentCanvas()->property("idealEdgeLengthModifier").toDouble());
    shapeNonOverlapPaddingSpinBox->setValue(diagramSceneTabWidget->currentCanvas()->property("shapeNonOverlapPadding").toInt());
    flowSeparationModifierSpinBox->setValue(diagramSceneTabWidget->currentCanvas()->property("flowSeparationModifier").toDouble());
}

void DiagramWindow::restore()
{
    fitVisibleButton->setChecked(false);
    restoreZoom();
}

void DiagramWindow::deleteSelection()
{
    diagramSceneTabWidget->currentCanvas()->deleteSelection();
}

void DiagramWindow::alignSelection(int alignType)
{
    if (!diagramSceneTabWidget->currentCanvas()->selectedItems().isEmpty())
    {
        diagramSceneTabWidget->currentCanvas()->alignSelection(alignType);
        arrange();
    }
}


void DiagramWindow::toFitVisible()
{
    diagramSceneTabWidget->currentDiagramView()->fitVisible();
        disconnect(diagramSceneTabWidget->currentCanvas(), SIGNAL(animationFinished()),
                   this, SLOT(toFitVisible()));
}

void DiagramWindow::setZoom()
{
    fitVisibleButton->setChecked(false);
    sceneScaleChanged(sceneScaleCombo->currentText());
}

void DiagramWindow::loadLayout()
{
    diagramSceneTabWidget->currentDiagramView()->loadLayout();
}

void DiagramWindow::saveLayout()
{
    diagramSceneTabWidget->currentDiagramView()->saveLayout();
}

void DiagramWindow::createMenus()
{
    QToolBar *layoutToolBar = addToolBar("Auto layout");

    QAction *arrangeAct = new QAction(tr("Arrange"), this);
    connect(arrangeAct, SIGNAL(triggered()), this, SLOT(arrange()));
    layoutToolBar->addAction(arrangeAct);

//    QAction *initArrangementAct = new QAction(tr("Init"), this);
//    connect(initArrangementAct, SIGNAL(triggered()), this, SLOT(initArrangement()));
//    layoutToolBar->addAction(initArrangementAct);

    fitVisibleButton = new QRadioButton(tr("Fit Visible"));
    layoutToolBar->addWidget(fitVisibleButton);
    connect(fitVisibleButton, SIGNAL(toggled(bool)), this, SLOT(fitVisible(bool)));
    fitVisibleButton->setChecked(false);

    QAction *loadLayoutAct = new QAction(tr("Reload layout"), this);
    connect(loadLayoutAct, SIGNAL(triggered()), this, SLOT(loadLayout()));
    layoutToolBar->addAction(loadLayoutAct);

    QAction *saveLayoutAct = new QAction(tr("Save layout"), this);
    connect(saveLayoutAct, SIGNAL(triggered()), this, SLOT(saveLayout()));
    layoutToolBar->addAction(saveLayoutAct);


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
        diagramSceneTabWidget->currentCanvas()->setProperty("layoutMode", Canvas::OrganicLayout);
    });
    connect(flowButton, &QRadioButton::toggled, [=](){
        diagramSceneTabWidget->currentCanvas()->setProperty("layoutMode", Canvas::FlowLayout);
    });
    connect(layeredButton, &QRadioButton::toggled, [=](){
        diagramSceneTabWidget->currentCanvas()->setProperty("layoutMode", Canvas::LayeredLayout);
    });

    organicButton->setChecked(true);
#endif

//    QDockWidget *layoutDock = new QDockWidget("Layouts");
//    //     layoutDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
//    layoutDock->setWidget(layoutModeBox);

//    addDockWidget(Qt::LeftDockWidgetArea, layoutDock);

    // ALIGNMENT

    QGroupBox * alignmentBox = new QGroupBox("Alignment");
    QPushButton *vertLeftButton = new QPushButton("Left");
    QPushButton *vertCentreButton = new QPushButton("Centre");
    QPushButton *vertRightButton = new QPushButton("Right");
    QPushButton *horiTopButton = new QPushButton("Top");
    QPushButton *horiCentreButton = new QPushButton("Middle");
    QPushButton *horiBottomButton = new QPushButton("Bottom");

    QGridLayout *alignmentLayout = new QGridLayout;
    alignmentLayout->addWidget(vertLeftButton, 0, 0);
    alignmentLayout->addWidget(vertCentreButton, 1, 0);
    alignmentLayout->addWidget(vertRightButton, 2, 0);
    alignmentLayout->addWidget(horiTopButton, 3, 0);
    alignmentLayout->addWidget(horiCentreButton, 4, 0);
    alignmentLayout->addWidget(horiBottomButton, 5, 0);
//    alignmentLayout->addStretch();
    alignmentBox->setLayout(alignmentLayout);

    QSignalMapper *alignSignalMapper = new QSignalMapper(this);
    alignSignalMapper->setMapping(vertLeftButton, (int) dunnart::ALIGN_LEFT);
    alignSignalMapper->setMapping(vertCentreButton, (int) dunnart::ALIGN_CENTER);
    alignSignalMapper->setMapping(vertRightButton, (int) dunnart::ALIGN_RIGHT);
    alignSignalMapper->setMapping(horiTopButton, (int) dunnart::ALIGN_TOP);
    alignSignalMapper->setMapping(horiCentreButton, (int) dunnart::ALIGN_MIDDLE);
    alignSignalMapper->setMapping(horiBottomButton, (int) dunnart::ALIGN_BOTTOM);

    connect(vertLeftButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));
    connect(vertCentreButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));
    connect(vertRightButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));
    connect(horiTopButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));
    connect(horiCentreButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));
    connect(horiBottomButton, SIGNAL(clicked()), alignSignalMapper, SLOT(map()));

    connect(alignSignalMapper, SIGNAL(mapped(int)),
            this, SLOT(alignSelection(int)));

    // ZOOM
    sceneScaleCombo = new QComboBox;
    QStringList scales;
    scales <<  tr("25%") << tr("33%") <<  tr("50%") << tr("75%") << tr("100%");
    sceneScaleCombo->addItems(scales);
    sceneScaleCombo->setCurrentIndex(4);

    // input validator
    QRegExp zoomRex("\\d{1,3}%?");
    QValidator *zoomValidator = new QRegExpValidator(zoomRex, sceneScaleCombo);
    QLineEdit *zoomEdit = new QLineEdit(sceneScaleCombo);
    zoomEdit->setValidator(zoomValidator);
    sceneScaleCombo->setLineEdit(zoomEdit);
    sceneScaleCombo->setInsertPolicy(QComboBox::NoInsert);

     connect(sceneScaleCombo, SIGNAL(currentIndexChanged(int)),
             this, SLOT(setZoom()));
     connect(sceneScaleCombo->lineEdit(), SIGNAL(returnPressed()),
             this, SLOT(setZoom()));

     for (int i = 0; i < diagramSceneTabWidget->count(); ++i)
         connect(diagramSceneTabWidget->diagramViewAt(i), SIGNAL(zoomChanged()),
                 this, SLOT(restoreZoom()));

    QToolBar *pointerToolbar = addToolBar("Zoom");
    pointerToolbar->addWidget(sceneScaleCombo);




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
        diagramSceneTabWidget->currentCanvas()->setProperty("idealEdgeLengthModifier", value);
    });
    connect(shapeNonOverlapPaddingSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int value){
        diagramSceneTabWidget->currentCanvas()->setProperty("shapeNonOverlapPadding", value);
    });
    connect(flowSeparationModifierSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value){
        diagramSceneTabWidget->currentCanvas()->setProperty("flowSeparationModifier", value);
    });
    connect(diagramSceneTabWidget, SIGNAL(currentChanged(int)), this, SLOT(restoreProperties()));

    restoreProperties();
#endif


    QVBoxLayout *controlsLayout = new QVBoxLayout;
//    controlsLayout->addWidget(layoutModeBox);
    controlsLayout->addWidget(alignmentBox);
//    controlsLayout->addWidget(propertiesBox);
    controlsLayout->addStretch();
    QWidget *controlsWidget = new QWidget;
    controlsWidget->setLayout(controlsLayout);

    QDockWidget *controlsDock = new QDockWidget("Layout");
    controlsDock->setWidget(controlsWidget);
    addDockWidget(Qt::LeftDockWidgetArea, controlsDock);


    // DELETE
    QAction *deleteAct = new QAction(tr("delete alignment"), this);
    deleteAct->setShortcut(QKeySequence::Delete);
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteSelection()));
    QToolBar *editToolBar = addToolBar("Edit");
    editToolBar->addAction(deleteAct);


    connect(diagramSceneTabWidget, SIGNAL(currentChanged(int)), this, SLOT(restore()));

}

