#include "diagramwindow.h"
#include "diagramscenetabwidget.h"
#include "diagramscene.h"
#include "libdunnartcanvas/graphlayout.h"


#include <QtWidgets>
#include <QSignalMapper>

using dunnart::Canvas;

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
    layoutModeVboxLayout->addStretch();
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

    QDockWidget *layoutDock = new QDockWidget("Layouts");
    //     layoutDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    layoutDock->setWidget(layoutModeBox);

    addDockWidget(Qt::LeftDockWidgetArea, layoutDock);

    // ALIGNMENT

    QGroupBox * alignmentBox = new QGroupBox("Alignment");
    QPushButton *vertLeftButton = new QPushButton("Left");
    QPushButton *vertCentreButton = new QPushButton("Centre");
    QPushButton *vertRightButton = new QPushButton("Right");
    QPushButton *horiTopButton = new QPushButton("Top");
    QPushButton *horiCentreButton = new QPushButton("Middle");
    QPushButton *horiBottomButton = new QPushButton("Bottom");

    QVBoxLayout *alignmentVboxLayout = new QVBoxLayout;
    alignmentVboxLayout->addWidget(vertLeftButton);
    alignmentVboxLayout->addWidget(vertCentreButton);
    alignmentVboxLayout->addWidget(vertRightButton);
    alignmentVboxLayout->addWidget(horiTopButton);
    alignmentVboxLayout->addWidget(horiCentreButton);
    alignmentVboxLayout->addWidget(horiBottomButton);
    alignmentVboxLayout->addStretch();
    alignmentBox->setLayout(alignmentVboxLayout);

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

    QDockWidget *alignDock = new QDockWidget("Alignment");
    //     layoutDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    alignDock->setWidget(alignmentBox);

    addDockWidget(Qt::LeftDockWidgetArea, alignDock);
}

