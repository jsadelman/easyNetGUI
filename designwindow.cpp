/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "arrow.h"
#include "diagramitem.h"
#include "diagramscene.h"
#include "diagramtextitem.h"
#include "designwindow.h"

#include <QtWidgets>

const int InsertTextButton = 10;

//! [0]
DesignWindow::DesignWindow(ObjectCatalogue *objectCatalogue, QWidget *parent)
 : objectCatalogue(objectCatalogue), QWidget(parent)
{
//    createActions();
//    createToolBox();
    createMenus();


    scene = new DiagramScene(itemMenu, objectCatalogue,  this);
    scene->setSceneRect(QRectF(0, 0, 1000, 1000));

//    connect(scene,SIGNAL(showObj(LazyNutObj*,LazyNutObjCatalogue*)),
//            this,SIGNAL(showObj(LazyNutObj*,LazyNutObjCatalogue*)));

    connect(this,SIGNAL(objectSelected(QString)),
            scene,SLOT(setSelected(QString)));
    connect(scene,SIGNAL(objectSelected(QString)),
            this,SIGNAL(objectSelected(QString)));
    connect(this,SIGNAL(savedLayoutToBeLoaded(QString)),
            scene,SLOT(savedLayoutToBeLoaded(QString)));
    connect(this,SIGNAL(saveLayout()),
            scene,SLOT(saveLayout()));
    connect(scene,SIGNAL(layoutSaveAttempted()),
            this,SIGNAL(layoutSaveAttempted()));


//    connect(scene, SIGNAL(itemInserted(DiagramItem*)),
//            this, SLOT(itemInserted(DiagramItem*)));
//    connect(scene, SIGNAL(textInserted(QGraphicsTextItem*)),
//            this, SLOT(textInserted(QGraphicsTextItem*)));
//    connect(scene, SIGNAL(itemSelected(QGraphicsItem*)),
//            this, SLOT(itemSelected(QGraphicsItem*)));
    createToolbars();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(pointerToolbar);
    view = new QGraphicsView(scene);

    layout->addWidget(view);
    view->setDragMode(QGraphicsView::RubberBandDrag);

    view->setRubberBandSelectionMode(Qt::ContainsItemShape);

    //QWidget *widget = new QWidget;
    //widget->setLayout(layout);
    setLayout(layout);
//    setCentralWidget(widget);
//    setWindowTitle(tr("Design Window"));
//    setUnifiedTitleAndToolBarOnMac(true);

    //createTestDiagram();
}

void DesignWindow::setObjCatalogue(ObjectCatalogue *catalogue)
{
    scene->setObjCatalogue(catalogue);
}

void DesignWindow::updateDiagramScene()
{
    //scene->syncToObjCatalogue();
}

void DesignWindow::dispatchObjectSelected(QString name)
{
    if (sender() == scene)
        emit objectSelected(name); // to objExplorer
    else
        scene->setSelected(name);
}
//! [0]

//! [1]
//void DesignWindow::backgroundButtonGroupClicked(QAbstractButton *button)
//{
//    QList<QAbstractButton *> buttons = backgroundButtonGroup->buttons();
//    foreach (QAbstractButton *myButton, buttons) {
//        if (myButton != button)
//            button->setChecked(false);
//    }
//    QString text = button->text();
//    if (text == tr("Blue Grid"))
//        scene->setBackgroundBrush(QPixmap(":/images/background1.png"));
//    else if (text == tr("White Grid"))
//        scene->setBackgroundBrush(QPixmap(":/images/background2.png"));
//    else if (text == tr("Gray Grid"))
//        scene->setBackgroundBrush(QPixmap(":/images/background3.png"));
//    else
//        scene->setBackgroundBrush(QPixmap(":/images/background4.png"));

//    scene->update();
//    view->update();
//}

//void DesignWindow::connectionButtonGroupClicked(int id)
//{
//    QList<QAbstractButton *> buttons = connectionButtonGroup->buttons();
//    foreach (QAbstractButton *button, buttons) {
//        if (connectionButtonGroup->button(id) != button)
//            button->setChecked(false);
//    }
//    scene->setMode(DiagramScene::InsertLine);
//    scene->setArrowTipType(Arrow::ArrowTipType(id));
//}

////! [1]

////! [2]
//void DesignWindow::layerButtonGroupClicked(int id)
//{
//    QList<QAbstractButton *> buttons = layerButtonGroup->buttons();
//    foreach (QAbstractButton *button, buttons) {
//        if (layerButtonGroup->button(id) != button)
//            button->setChecked(false);
//    }
//    if (id == InsertTextButton) {
//        scene->setMode(DiagramScene::InsertText);
//    } else {
//        scene->setItemType(DiagramItem::DiagramType(id));
//        scene->setMode(DiagramScene::InsertItem);
//    }
//}
//! [2]

//! [3]
void DesignWindow::deleteItem()
{
    foreach (QGraphicsItem *item, scene->selectedItems()) {
        if (item->type() == Arrow::Type) {
            scene->removeItem(item);
            Arrow *arrow = qgraphicsitem_cast<Arrow *>(item);
            arrow->getStartItem()->removeArrow(arrow);
            arrow->getEndItem()->removeArrow(arrow);
            delete item;
        }
    }

    foreach (QGraphicsItem *item, scene->selectedItems()) {
         if (item->type() == DiagramItem::Type)
             qgraphicsitem_cast<DiagramItem *>(item)->removeArrows();
         scene->removeItem(item);
         delete item;
     }
}
//! [3]

//! [4]
//void DesignWindow::pointerGroupClicked(int)
//{
//    scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));
//}
////! [4]

////! [5]
//void DesignWindow::bringToFront()
//{
//    if (scene->selectedItems().isEmpty())
//        return;

//    QGraphicsItem *selectedItem = scene->selectedItems().first();
//    QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

//    qreal zValue = 0;
//    foreach (QGraphicsItem *item, overlapItems) {
//        if (item->zValue() >= zValue && item->type() == DiagramItem::Type)
//            zValue = item->zValue() + 0.1;
//    }
//    selectedItem->setZValue(zValue);
//}
////! [5]

////! [6]
//void DesignWindow::sendToBack()
//{
//    if (scene->selectedItems().isEmpty())
//        return;

//    QGraphicsItem *selectedItem = scene->selectedItems().first();
//    QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

//    qreal zValue = 0;
//    foreach (QGraphicsItem *item, overlapItems) {
//        if (item->zValue() <= zValue && item->type() == DiagramItem::Type)
//            zValue = item->zValue() - 0.1;
//    }
//    selectedItem->setZValue(zValue);
//}
//! [6]

//! [7]
//void DesignWindow::itemInserted(DiagramItem *item)
//{
//    pointerTypeGroup->button(int(DiagramScene::MoveItem))->setChecked(true);
//    scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));
//    layerButtonGroup->button(int(item->diagramType()))->setChecked(false);
//}
//! [7]

//! [8]
//void DesignWindow::textInserted(QGraphicsTextItem *)
//{
//    layerButtonGroup->button(InsertTextButton)->setChecked(false);
//    scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));
//}
//! [8]

//! [9]
//void DesignWindow::currentFontChanged(const QFont &)
//{
//    handleFontChange();
//}
////! [9]

////! [10]
//void DesignWindow::fontSizeChanged(const QString &)
//{
//    handleFontChange();
//}
//! [10]

//! [11]
void DesignWindow::sceneScaleChanged(const QString &scale)
{
    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
    QMatrix oldMatrix = view->matrix();
    view->resetMatrix();
    view->translate(oldMatrix.dx(), oldMatrix.dy());
    view->scale(newScale, newScale);
}
//! [11]

//! [12]
//void DesignWindow::textColorChanged()
//{
//    textAction = qobject_cast<QAction *>(sender());
//    fontColorToolButton->setIcon(createColorToolButtonIcon(
//                                     ":/images/textpointer.png",
//                                     qvariant_cast<QColor>(textAction->data())));
//    textButtonTriggered();
//}
////! [12]

////! [13]
//void DesignWindow::itemColorChanged()
//{
//    fillAction = qobject_cast<QAction *>(sender());
//    fillColorToolButton->setIcon(createColorToolButtonIcon(
//                                     ":/images/floodfill.png",
//                                     qvariant_cast<QColor>(fillAction->data())));
//    fillButtonTriggered();
//}
////! [13]

////! [14]
//void DesignWindow::lineColorChanged()
//{
//    lineAction = qobject_cast<QAction *>(sender());
//    lineColorToolButton->setIcon(createColorToolButtonIcon(
//                                     ":/images/linecolor.png",
//                                     qvariant_cast<QColor>(lineAction->data())));
//    lineButtonTriggered();
//}
////! [14]

////! [15]
//void DesignWindow::textButtonTriggered()
//{
//    scene->setTextColor(qvariant_cast<QColor>(textAction->data()));
//}
////! [15]

////! [16]
//void DesignWindow::fillButtonTriggered()
//{
//    scene->setItemColor(qvariant_cast<QColor>(fillAction->data()));
//}
////! [16]

////! [17]
//void DesignWindow::lineButtonTriggered()
//{
//    scene->setLineColor(qvariant_cast<QColor>(lineAction->data()));
//}
////! [17]

////! [18]
//void DesignWindow::handleFontChange()
//{
//    QFont font = fontCombo->currentFont();
//    font.setPointSize(fontSizeCombo->currentText().toInt());
//    font.setWeight(boldAction->isChecked() ? QFont::Bold : QFont::Normal);
//    font.setItalic(italicAction->isChecked());
//    font.setUnderline(underlineAction->isChecked());

//    scene->setFont(font);
//}
//! [18]

//! [19]
//void DesignWindow::itemSelected(QGraphicsItem *item)
//{
//    DiagramTextItem *textItem =
//    qgraphicsitem_cast<DiagramTextItem *>(item);

//    QFont font = textItem->font();
//    fontCombo->setCurrentFont(font);
//    fontSizeCombo->setEditText(QString().setNum(font.pointSize()));
//    boldAction->setChecked(font.weight() == QFont::Bold);
//    italicAction->setChecked(font.italic());
//    underlineAction->setChecked(font.underline());
//}
//! [19]

//! [20]
//void DesignWindow::about()
//{
//    QMessageBox::about(this, tr("About Diagram Scene"),
//                       tr("The <b>Diagram Scene</b> example shows "
//                          "use of the graphics framework."));
//}

void DesignWindow::createTestDiagram()
{
    // two boxes and an arrow
   scene->setMode(DiagramScene::InsertItem);
    scene->setItemType(DiagramItem::Layer);
    DiagramItem* startItem = new DiagramItem(DiagramItem::Layer, "A", itemMenu);
    startItem->setBrush(scene->itemColor());
    scene->addItem(startItem);
    startItem->setPos(QPointF(100,100));
    emit scene->itemInserted(startItem);
    view->centerOn(QPointF(100,100));
    DiagramItem* endItem = new DiagramItem(DiagramItem::Layer, "B", itemMenu);
    endItem->setBrush(scene->itemColor());
    scene->addItem(endItem);
    endItem->setPos(QPointF(400,300));
    emit scene->itemInserted(endItem);

    scene->setMode(DiagramScene::InsertLine);
    Arrow *arrow = new Arrow("AB1", startItem, endItem, Arrow::Inhibitory);
    arrow->setColor(scene->lineColor());
    startItem->addArrow(arrow);
    endItem->addArrow(arrow);
    arrow->setZValue(-1000.0);
    scene->addItem(arrow);
    arrow->updatePosition();

    arrow = new Arrow("AB2",startItem, endItem);
    arrow->setColor(scene->lineColor());
    startItem->addArrow(arrow);
    endItem->addArrow(arrow);
    arrow->setZValue(-1000.0);
    scene->addItem(arrow);
    arrow->updatePosition();
    scene->setMode(DiagramScene::MoveItem);



}
//! [20]

//! [21]
//void DesignWindow::createToolBox()
//{
//    layerButtonGroup = new QButtonGroup(this);
//    layerButtonGroup->setExclusive(false);
//    connect(layerButtonGroup, SIGNAL(buttonClicked(int)),
//            this, SLOT(layerButtonGroupClicked(int)));
//    QGridLayout *layout = new QGridLayout;
////    layout->addWidget(createCellWidget(tr("Conditional"), DiagramItem::Conditional), 0, 0);
//    layout->addWidget(createCellWidget(tr("Layer"), DiagramItem::Layer),0, 0);
//    layout->addWidget(createCellWidget(tr("Diamond"), DiagramItem::Diamond),0, 1);

////    layout->addWidget(createCellWidget(tr("Input/Output"), DiagramItem::Io), 1, 0);
////! [21]

////    QToolButton *textButton = new QToolButton;
////    textButton->setCheckable(true);
////    layerButtonGroup->addButton(textButton, InsertTextButton);
////    textButton->setIcon(QIcon(QPixmap(":/images/textpointer.png")));
////    textButton->setIconSize(QSize(50, 50));
////    QGridLayout *textLayout = new QGridLayout;
////    textLayout->addWidget(textButton, 0, 0, Qt::AlignHCenter);
////    textLayout->addWidget(new QLabel(tr("Text")), 1, 0, Qt::AlignCenter);
////    QWidget *textWidget = new QWidget;
////    textWidget->setLayout(textLayout);
////    layout->addWidget(textWidget, 1, 0);

//    layout->setRowStretch(1, 10);
//    layout->setColumnStretch(2, 10);

//    QWidget *itemWidget = new QWidget;
//    itemWidget->setLayout(layout);

//    backgroundButtonGroup = new QButtonGroup(this);
//    connect(backgroundButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
//            this, SLOT(backgroundButtonGroupClicked(QAbstractButton*)));

//    QGridLayout *backgroundLayout = new QGridLayout;
//    backgroundLayout->addWidget(createBackgroundCellWidget(tr("Blue Grid"),
//                                                           ":/images/background1.png"), 0, 0);
//    backgroundLayout->addWidget(createBackgroundCellWidget(tr("White Grid"),
//                                                           ":/images/background2.png"), 0, 1);
//    backgroundLayout->addWidget(createBackgroundCellWidget(tr("Gray Grid"),
//                                                           ":/images/background3.png"), 1, 0);
//    backgroundLayout->addWidget(createBackgroundCellWidget(tr("No Grid"),
//                                                           ":/images/background4.png"), 1, 1);

//    backgroundLayout->setRowStretch(2, 10);
//    backgroundLayout->setColumnStretch(2, 10);

//    QWidget *backgroundWidget = new QWidget;
//    backgroundWidget->setLayout(backgroundLayout);

//    // connections
//    connectionButtonGroup = new QButtonGroup(this);
//    connect(connectionButtonGroup,SIGNAL(buttonClicked(int)),
//            this, SLOT(connectionButtonGroupClicked(int)));
//    QGridLayout *connectionLayout = new QGridLayout;
//    connectionLayout->addWidget(createConnectionCellWidget(tr("Excitatory"),
//                                                           ":/images/excitatory.png", Arrow::Excitatory), 0, 0);
//    connectionLayout->addWidget(createConnectionCellWidget(tr("Inhibitory"),
//                                                           ":/images/inhibitory.png", Arrow::Inhibitory), 0, 1);
//    connectionLayout->setRowStretch(1, 10);
//    connectionLayout->setColumnStretch(2, 10);

//    QWidget *connectionWidget = new QWidget;
//    connectionWidget->setLayout(connectionLayout);

////! [22]
//    toolBox = new QToolBox;
//    toolBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored));
//    toolBox->setMinimumWidth(itemWidget->sizeHint().width());
//    toolBox->addItem(itemWidget, tr("Layers"));
//    //toolBox->addItem(backgroundWidget, tr("Backgrounds"));
//    toolBox->addItem(connectionWidget, tr("Connections"));

//}
////! [22]

////! [23]
//void DesignWindow::createActions()
//{
//    toFrontAction = new QAction(QIcon(":/images/bringtofront.png"),
//                                tr("Bring to &Front"), this);
//    toFrontAction->setShortcut(tr("Ctrl+F"));
//    toFrontAction->setStatusTip(tr("Bring item to front"));
//    connect(toFrontAction, SIGNAL(triggered()), this, SLOT(bringToFront()));
////! [23]

//    sendBackAction = new QAction(QIcon(":/images/sendtoback.png"), tr("Send to &Back"), this);
//    sendBackAction->setShortcut(tr("Ctrl+B"));
//    sendBackAction->setStatusTip(tr("Send item to back"));
//    connect(sendBackAction, SIGNAL(triggered()), this, SLOT(sendToBack()));

//    deleteAction = new QAction(QIcon(":/images/delete.png"), tr("&Delete"), this);
//    deleteAction->setShortcut(tr("Delete"));
//    deleteAction->setStatusTip(tr("Delete item from diagram"));
//    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

//    exitAction = new QAction(tr("E&xit"), this);
//    exitAction->setShortcuts(QKeySequence::Quit);
//    exitAction->setStatusTip(tr("Quit Scenediagram example"));
//    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

//    boldAction = new QAction(tr("Bold"), this);
//    boldAction->setCheckable(true);
//    QPixmap pixmap(":/images/bold.png");
//    boldAction->setIcon(QIcon(pixmap));
//    boldAction->setShortcut(tr("Ctrl+B"));
//    connect(boldAction, SIGNAL(triggered()), this, SLOT(handleFontChange()));

//    italicAction = new QAction(QIcon(":/images/italic.png"), tr("Italic"), this);
//    italicAction->setCheckable(true);
//    italicAction->setShortcut(tr("Ctrl+I"));
//    connect(italicAction, SIGNAL(triggered()), this, SLOT(handleFontChange()));

//    underlineAction = new QAction(QIcon(":/images/underline.png"), tr("Underline"), this);
//    underlineAction->setCheckable(true);
//    underlineAction->setShortcut(tr("Ctrl+U"));
//    connect(underlineAction, SIGNAL(triggered()), this, SLOT(handleFontChange()));

//    aboutAction = new QAction(tr("A&bout"), this);
//    aboutAction->setShortcut(tr("Ctrl+B"));
//    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
//}

//! [24]
void DesignWindow::createMenus()
{
//    fileMenu = menuBar()->addMenu(tr("&File"));
//    fileMenu->addAction(exitAction);

    //itemMenu = menuBar()->addMenu(tr("&Item"));
    itemMenu = new QMenu(this);
//    itemMenu->addAction(deleteAction);
//    itemMenu->addSeparator();
//    itemMenu->addAction(toFrontAction);
//    itemMenu->addAction(sendBackAction);

//    aboutMenu = menuBar()->addMenu(tr("&Help"));
//    aboutMenu->addAction(aboutAction);
}
//! [24]

//! [25]
void DesignWindow::createToolbars()
{
////! [25]
//    editToolBar = addToolBar(tr("Edit"));
//    editToolBar->addAction(deleteAction);
//    editToolBar->addAction(toFrontAction);
//    editToolBar->addAction(sendBackAction);

//    fontCombo = new QFontComboBox();
//    connect(fontCombo, SIGNAL(currentFontChanged(QFont)),
//            this, SLOT(currentFontChanged(QFont)));

//    fontSizeCombo = new QComboBox;
//    fontSizeCombo->setEditable(true);
//    for (int i = 8; i < 30; i = i + 2)
//        fontSizeCombo->addItem(QString().setNum(i));
//    QIntValidator *validator = new QIntValidator(2, 64, this);
//    fontSizeCombo->setValidator(validator);
//    connect(fontSizeCombo, SIGNAL(currentIndexChanged(QString)),
//            this, SLOT(fontSizeChanged(QString)));

//    fontColorToolButton = new QToolButton;
//    fontColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
//    fontColorToolButton->setMenu(createColorMenu(SLOT(textColorChanged()), Qt::black));
//    textAction = fontColorToolButton->menu()->defaultAction();
//    fontColorToolButton->setIcon(createColorToolButtonIcon(":/images/textpointer.png", Qt::black));
//    fontColorToolButton->setAutoFillBackground(true);
//    connect(fontColorToolButton, SIGNAL(clicked()),
//            this, SLOT(textButtonTriggered()));

////! [26]
//    fillColorToolButton = new QToolButton;
//    fillColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
//    fillColorToolButton->setMenu(createColorMenu(SLOT(itemColorChanged()), Qt::white));
//    fillAction = fillColorToolButton->menu()->defaultAction();
//    fillColorToolButton->setIcon(createColorToolButtonIcon(
//                                     ":/images/floodfill.png", Qt::white));
//    connect(fillColorToolButton, SIGNAL(clicked()),
//            this, SLOT(fillButtonTriggered()));
////! [26]

//    lineColorToolButton = new QToolButton;
//    lineColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
//    lineColorToolButton->setMenu(createColorMenu(SLOT(lineColorChanged()), Qt::black));
//    lineAction = lineColorToolButton->menu()->defaultAction();
//    lineColorToolButton->setIcon(createColorToolButtonIcon(
//                                     ":/images/linecolor.png", Qt::black));
//    connect(lineColorToolButton, SIGNAL(clicked()),
//            this, SLOT(lineButtonTriggered()));

//    textToolBar = addToolBar(tr("Font"));
//    textToolBar->addWidget(fontCombo);
//    textToolBar->addWidget(fontSizeCombo);
//    textToolBar->addAction(boldAction);
//    textToolBar->addAction(italicAction);
//    textToolBar->addAction(underlineAction);

//    colorToolBar = addToolBar(tr("Color"));
//    colorToolBar->addWidget(fontColorToolButton);
//    colorToolBar->addWidget(fillColorToolButton);
//    colorToolBar->addWidget(lineColorToolButton);

//    QToolButton *pointerButton = new QToolButton;
//    pointerButton->setCheckable(true);
//    pointerButton->setChecked(true);
//    pointerButton->setIcon(QIcon(":/images/pointer.png"));
//    QToolButton *linePointerButton = new QToolButton;
//    linePointerButton->setCheckable(true);
//    linePointerButton->setIcon(QIcon(":/images/linepointer.png"));

//    pointerTypeGroup = new QButtonGroup(this);
//    pointerTypeGroup->addButton(pointerButton, int(DiagramScene::MoveItem));
//    pointerTypeGroup->addButton(linePointerButton, int(DiagramScene::InsertLine));
//    connect(pointerTypeGroup, SIGNAL(buttonClicked(int)),
//            this, SLOT(pointerGroupClicked(int)));

    sceneScaleCombo = new QComboBox;
    QStringList scales;
    scales << tr("50%") << tr("75%") << tr("100%") << tr("125%") << tr("150%");
    sceneScaleCombo->addItems(scales);
    sceneScaleCombo->setCurrentIndex(2);
    connect(sceneScaleCombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(sceneScaleChanged(QString)));



    pointerToolbar = new QToolBar(tr("zoom"));
//    pointerToolbar->addWidget(pointerButton);
//    pointerToolbar->addWidget(linePointerButton);
    pointerToolbar->addWidget(sceneScaleCombo);
////! [27]
}
////! [27]

////! [28]
//QWidget *DesignWindow::createBackgroundCellWidget(const QString &text, const QString &image)
//{
//    QToolButton *button = new QToolButton;
//    button->setText(text);
//    button->setIcon(QIcon(image));
//    button->setIconSize(QSize(50, 50));
//    button->setCheckable(true);
//    backgroundButtonGroup->addButton(button);

//    QGridLayout *layout = new QGridLayout;
//    layout->addWidget(button, 0, 0, Qt::AlignHCenter);
//    layout->addWidget(new QLabel(text), 1, 0, Qt::AlignCenter);

//    QWidget *widget = new QWidget;
//    widget->setLayout(layout);

//    return widget;
//}
////! [28]

////! [29]
//QWidget *DesignWindow::createCellWidget(const QString &text, DiagramItem::DiagramType type)
//{

//    DiagramItem item(type, itemMenu);
//    QIcon icon(item.image());

//    QToolButton *button = new QToolButton;
//    button->setIcon(icon);
//    button->setIconSize(QSize(50, 50));
//    button->setCheckable(true);
//    layerButtonGroup->addButton(button, int(type));

//    QGridLayout *layout = new QGridLayout;
//    layout->addWidget(button, 0, 0, Qt::AlignHCenter);
//    layout->addWidget(new QLabel(text), 1, 0, Qt::AlignCenter);

//    QWidget *widget = new QWidget;
//    widget->setLayout(layout);

//    return widget;
//}

//QWidget *DesignWindow::createConnectionCellWidget(const QString &text, const QString &image, Arrow::ArrowTipType type)
//{
//    QToolButton *button = new QToolButton;
//    button->setText(text);
//    button->setIcon(QIcon(image));
//    button->setIconSize(QSize(50, 50));
//    button->setCheckable(true);
//    connectionButtonGroup->addButton(button, int(type));

//    QGridLayout *layout = new QGridLayout;
//    layout->addWidget(button, 0, 0, Qt::AlignHCenter);
//    layout->addWidget(new QLabel(text), 1, 0, Qt::AlignCenter);

//    QWidget *widget = new QWidget;
//    widget->setLayout(layout);

//    return widget;

//}

////! [29]

////! [30]
//QMenu *DesignWindow::createColorMenu(const char *slot, QColor defaultColor)
//{
//    QList<QColor> colors;
//    colors << Qt::black << Qt::white << Qt::red << Qt::blue << Qt::yellow;
//    QStringList names;
//    names << tr("black") << tr("white") << tr("red") << tr("blue")
//          << tr("yellow");

//    QMenu *colorMenu = new QMenu(this);
//    for (int i = 0; i < colors.count(); ++i) {
//        QAction *action = new QAction(names.at(i), this);
//        action->setData(colors.at(i));
//        action->setIcon(createColorIcon(colors.at(i)));
//        connect(action, SIGNAL(triggered()), this, slot);
//        colorMenu->addAction(action);
//        if (colors.at(i) == defaultColor)
//            colorMenu->setDefaultAction(action);
//    }
//    return colorMenu;
//}
////! [30]

////! [31]
//QIcon DesignWindow::createColorToolButtonIcon(const QString &imageFile, QColor color)
//{
//    QPixmap pixmap(50, 80);
//    pixmap.fill(Qt::transparent);
//    QPainter painter(&pixmap);
//    QPixmap image(imageFile);
//    // Draw icon centred horizontally on button.
//    QRect target(4, 0, 42, 43);
//    QRect source(0, 0, 42, 43);
//    painter.fillRect(QRect(0, 60, 50, 80), color);
//    painter.drawPixmap(target, image, source);

//    return QIcon(pixmap);
//}
////! [31]

////! [32]
//QIcon DesignWindow::createColorIcon(QColor color)
//{
//    QPixmap pixmap(20, 20);
//    QPainter painter(&pixmap);
//    painter.setPen(Qt::NoPen);
//    painter.fillRect(QRect(0, 0, 20, 20), color);

//    return QIcon(pixmap);
//}
////! [32]
