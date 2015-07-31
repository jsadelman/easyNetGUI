#include "diagramwindow.h"
#include "diagramscenetabwidget.h"
#include "diagramscene.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/canvasview.h"


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
}

void DiagramWindow::arrange()
{
    diagramSceneTabWidget->currentCanvas()->layout()->clearFixedList();
    rearrange();
}


void DiagramWindow::rearrange()
{
    diagramSceneTabWidget->currentCanvas()->layout()->runDirect(true);
    diagramSceneTabWidget->currentDiagramScene()->processLayoutUpdateEvent();
}

void DiagramWindow::sceneScaleChanged(const QString &scale)
{
    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
    QGraphicsView *view = diagramSceneTabWidget->currentCanvasView();
    QMatrix oldMatrix = view->matrix();
    view->resetMatrix();
    view->translate(oldMatrix.dx(), oldMatrix.dy());
    view->scale(newScale, newScale);
}

void DiagramWindow::recoverZoom()
{
    QGraphicsView *view = diagramSceneTabWidget->currentCanvasView();
    double m11 = view->transform().m11();
    QString zoomText;
    zoomText.setNum((int)(100 * m11)).append("\%");
    sceneScaleCombo->setCurrentIndex(sceneScaleCombo->findText(zoomText));
}

void DiagramWindow::recoverProperties()
{
    idealEdgeLengthModifierSpinBox->setValue(diagramSceneTabWidget->currentCanvas()->property("idealEdgeLengthModifier").toDouble());
    shapeNonOverlapPaddingSpinBox->setValue(diagramSceneTabWidget->currentCanvas()->property("shapeNonOverlapPadding").toInt());
    flowSeparationModifierSpinBox->setValue(diagramSceneTabWidget->currentCanvas()->property("flowSeparationModifier").toDouble());
}

void DiagramWindow::deleteSelection()
{
    qDebug() << "delete selection";
    diagramSceneTabWidget->currentCanvas()->deleteSelection();
}



void DiagramWindow::createMenus()
{
    QAction *arrangeAct = new QAction(tr("&Arrange"), this);
    connect(arrangeAct, SIGNAL(triggered()), this, SLOT(arrange()));
    QToolBar *layoutToolBar = addToolBar("Auto layout");
    layoutToolBar->addAction(arrangeAct);

    QAction *rearrangeAct = new QAction(tr("&Rearrange"), this);
    connect(rearrangeAct, SIGNAL(triggered()), this, SLOT(rearrange()));
    layoutToolBar->addAction(rearrangeAct);

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
    alignmentLayout->addWidget(vertCentreButton, 0, 1);
    alignmentLayout->addWidget(vertRightButton, 0, 2);
    alignmentLayout->addWidget(horiTopButton, 1, 0);
    alignmentLayout->addWidget(horiCentreButton, 1, 1);
    alignmentLayout->addWidget(horiBottomButton, 1, 2);
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
            diagramSceneTabWidget->currentCanvas(), SLOT(alignSelection(int)));

//    QDockWidget *alignDock = new QDockWidget("Alignment");
//    //     layoutDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
//    alignDock->setWidget(alignmentBox);

//    addDockWidget(Qt::LeftDockWidgetArea, alignDock);

    // ZOOM
    sceneScaleCombo = new QComboBox;
    QStringList scales;
    scales << tr("50%") << tr("75%") << tr("100%") << tr("125%") << tr("150%");
    sceneScaleCombo->addItems(scales);
    sceneScaleCombo->setCurrentIndex(2);
    connect(sceneScaleCombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(sceneScaleChanged(QString)));
//    connect(diagramSceneTabWidget, &QTabWidget::currentChanged, [=](){
//        sceneScaleChanged(sceneScaleCombo->currentText());
//    });
    connect(diagramSceneTabWidget, SIGNAL(currentChanged(int)), this, SLOT(recoverZoom()));

    QToolBar *pointerToolbar = addToolBar("Zoom");
    pointerToolbar->addWidget(sceneScaleCombo);


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
    connect(diagramSceneTabWidget, SIGNAL(currentChanged(int)), this, SLOT(recoverProperties()));

    recoverProperties();

    QVBoxLayout *controlsLayout = new QVBoxLayout;
    controlsLayout->addWidget(layoutModeBox);
    controlsLayout->addWidget(alignmentBox);
    controlsLayout->addWidget(propertiesBox);
    controlsLayout->addStretch();
    QWidget *controlsWidget = new QWidget;
    controlsWidget->setLayout(controlsLayout);

    QDockWidget *controlsDock = new QDockWidget("Layout Controls");
    controlsDock->setWidget(controlsWidget);
    addDockWidget(Qt::LeftDockWidgetArea, controlsDock);


    // DELETE
    QAction *deleteAct = new QAction(tr("delete"), this);
    deleteAct->setShortcut(QKeySequence::Delete);
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteSelection()));
    QToolBar *editToolBar = addToolBar("Edit");
    editToolBar->addAction(deleteAct);

}

