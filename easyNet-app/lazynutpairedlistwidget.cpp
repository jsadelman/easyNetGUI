#include "lazynutpairedlistwidget.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"


#include <QtWidgets>
#include <QListWidget>
#include <QDebug>


LazyNutPairedListWidget::LazyNutPairedListWidget(QString getListCmd, QWidget *parent) :
    getListCmd(getListCmd), QFrame(parent)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setFrameShape(QFrame::Panel);
    setFrameShape(QFrame::Panel);
//    editFrame = new QFrame;
//    editFrame->setFrameShape(QFrame::Panel);
//    editFrame->setFrameShape(QFrame::Panel);
//    setCentralWidget(editFrame);


    factorList = new QListWidget(this);
    factorList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectedList = new QListWidget(this);
    selectedList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // debug
//    connect(this, &LazyNutPairedListWidget::valueChanged,
//            [=](){qDebug() << "LazyNutPairedListWidget" <<  getValue();});
}


void LazyNutPairedListWidget::setGetListCmd(QString cmd)
{
    getListCmd = cmd;
}

void LazyNutPairedListWidget::getList(QString cmd)
{
//    buildList(QStringList({"(badger snake)", "mushroom", "(snake mole)"}));

    getListCmd = cmd.isEmpty() ? getListCmd : cmd;
    if (!getListCmd.isEmpty())
    {
        LazyNutJobParam *param = new LazyNutJobParam;
        param->logMode &= ECHO_INTERPRETER; // debug purpose
        param->cmdList = QStringList({QString("xml %1").arg(getListCmd)});
        param->answerFormatterType = AnswerFormatterType::ListOfValues;
        param->setAnswerReceiver(this, SLOT(buildList(QStringList)));
        SessionManager::instance()->setupJob(param, sender());
    }
    else
        emit listReady();
}

void LazyNutPairedListWidget::setValue(QStringList list)
{
    selectedList->selectAll();
    moveItems(selectedList, factorList, selectedList->selectedItems());
    QList<QListWidgetItem*> selectedItems;
    foreach(QString factor, list)
    {
        if (!factor.isEmpty())
            selectedItems.append(factorList->findItems(factor, Qt::MatchFixedString|Qt::MatchCaseSensitive));
    }
    moveItems(factorList, selectedList, selectedItems);
//    emit valueChanged();
}

QStringList LazyNutPairedListWidget::getValue()
{
    QStringList value;
    for(int row = 0; row < selectedList->count(); ++row)
        value.append(selectedList->item(row)->text());
    return value;
}

void LazyNutPairedListWidget::buildList(QStringList list)
{
//    showHideAct = new QAction("Hide", this);
//    connect(showHideAct, SIGNAL(triggered()), this, SLOT(showHide()));
//    QToolBar *showHideToolBar = addToolBar("");
//    showHideToolBar->addAction(showHideAct);

    factorList->addItems(list);
    QGridLayout *gridLayout = new QGridLayout;
    QPushButton *addButton = new QPushButton("==>", this);
    addButton->setToolTip("Add to selected factors");
    connect(addButton, SIGNAL(clicked()), this, SLOT(addItems()));
    QPushButton *removeButton = new QPushButton("<==", this);
    removeButton->setToolTip("Remove from selected factors");
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeItems()));
    QLabel *availableLabel = new QLabel("Available factors");
    QLabel *selectedLabel = new QLabel("Selected factors");
    gridLayout->addWidget(availableLabel, 0,0,Qt::AlignHCenter);
    gridLayout->addWidget(selectedLabel, 0,2,Qt::AlignHCenter);
//    editFrameLayout->setRowStretch(0,1);
    gridLayout->addWidget(factorList, 1,0,3,1);
    gridLayout->addWidget(selectedList, 1,2,3,1);
    gridLayout->addWidget(addButton,1,1);
    gridLayout->addWidget(removeButton,2,1);
//    editFrame->setLayout(gridLayout);
    setLayout(gridLayout);

//    frameHeight = editFrame->height();
//    editFrame->setMaximumHeight(editFrame->height());
    emit listReady();
}

//void LazyNutPairedListWidget::showHide()
//{
//    if (editFrame->isVisible())
//    {
//        editFrame->hide();
////        editFrame->resize(editFrame->width(),0);
//        showHideAct->setText("Show");
//    }
//    else
//    {
//        editFrame->show();
////        editFrame->resize(editFrame->width(),frameHeight);
//        showHideAct->setText("Hide");
//    }
//    emit sizeChanged();
//}

void LazyNutPairedListWidget::addItems()
{
    moveItems(factorList, selectedList, factorList->selectedItems());
    emit valueChanged();
}

void LazyNutPairedListWidget::removeItems()
{
    moveItems(selectedList, factorList, selectedList->selectedItems());
    emit valueChanged();
}

void LazyNutPairedListWidget::moveItems(QListWidget *fromList, QListWidget *toList, QList<QListWidgetItem *> items)
{
    foreach(QListWidgetItem* item, items)
        toList->addItem(fromList->takeItem(fromList->row(item)));

    fromList->sortItems();
    toList->sortItems();
}

