#include "pairedlistwidget.h"
#include "hidefromlistmodel.h"

#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QDebug>
#include <QLineEdit>

PairedListWidget::PairedListWidget(QAbstractItemModel *listModel, int relevantColumn, QWidget *parent)
    : listModel(listModel), relevantColumn(relevantColumn), QFrame(parent)
{
    if (listModel)
        buildWidget();
}

void PairedListWidget::setValue(QStringList selectedList)
{
    notSelectedModel->showAll();
    selectedWidget->clear();

    foreach (QString item, selectedList)
    {
        QModelIndexList matchList = notSelectedModel->match(notSelectedModel->index(0,0), Qt::DisplayRole, item, 1, Qt::MatchExactly);
        if (!matchList.isEmpty())
        {
            notSelectedModel->hideItem(item);
            selectedWidget->addItem(item);
        }
    }
    emit valueChanged();
}

QStringList PairedListWidget::getValue()
{
    QStringList list;
    if (listModel)
    {
        for(int row = 0; row < selectedWidget->count(); ++row)
            list.append(selectedWidget->item(row)->text());
    }
    return list;
}

void PairedListWidget::addSelected()
{
    if (notSelectedView->selectionModel()->selectedIndexes().isEmpty())
        return;
    while (!notSelectedView->selectionModel()->selectedIndexes().isEmpty())
    {
        QString item = notSelectedModel->data(notSelectedView->selectionModel()->selectedIndexes().first()).toString();
        notSelectedModel->hideItem(item);
        selectedWidget->addItem(item);
    }
    emit valueChanged();
}

void PairedListWidget::removeSelected()
{
    if (selectedWidget->selectedItems().isEmpty())
        return;
    while (!selectedWidget->selectedItems().isEmpty())
    {
        QString item = selectedWidget->selectedItems().first()->text();
        delete selectedWidget->takeItem(selectedWidget->row(selectedWidget->selectedItems().first()));
        notSelectedModel->showItem(item);
    }
    emit valueChanged();
}

void PairedListWidget::buildWidget()
{
    notSelectedModel = new HideFromListModel(this);
    notSelectedModel->setSourceModel(listModel);
    notSelectedModel->setRelevantColumn(relevantColumn);
    notSelectedModel->sort(0);

    QGridLayout *gridLayout = new QGridLayout;
    notSelectedView = new QListView(this);
    notSelectedView->setModel(notSelectedModel);
    notSelectedView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    selectedWidget = new QListWidget(this);
    selectedWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectedWidget->setDragDropMode(QAbstractItemView::InternalMove);
    connect(selectedWidget->model(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SIGNAL(valueChanged()));

    searchEdit = new QLineEdit(this);
    connect(searchEdit, &QLineEdit::textEdited, [=](QString text)
    {
        // first try case-sensitive search
        QModelIndexList indexList = notSelectedModel->match(notSelectedModel->index(0,0), Qt::DisplayRole, text, 1, Qt::MatchCaseSensitive);
        if (indexList.isEmpty())
            // no joy -- try case-insensitive search
            indexList = notSelectedModel->match(notSelectedModel->index(0,0), Qt::DisplayRole, text);
        if (!indexList.isEmpty())
            notSelectedView->setCurrentIndex(indexList.at(0));
    });
    connect(searchEdit, &QLineEdit::returnPressed, [=]()
    {
        addSelected();
        searchEdit->clear();
    });

    QHBoxLayout *searchLayout = new QHBoxLayout;
    QLabel *searchLabel = new QLabel("Search:");
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchEdit);


    addButton = new QPushButton("==>", this);
    addButton->setToolTip("Add to selected factors");
    connect(addButton, SIGNAL(clicked()), this, SLOT(addSelected()));
    connect(notSelectedView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(addSelected()));
    removeButton = new QPushButton("<==", this);
    removeButton->setToolTip("Remove from selected factors");
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
    connect(selectedWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(removeSelected()));
    notSelectedLabel = new QLabel("Available factors");
    selectedLabel = new QLabel("Selected factors");
//    gridLayout->addWidget(notSelectedLabel, 0,0,Qt::AlignHCenter);
    gridLayout->addLayout(searchLayout, 0,0,Qt::AlignHCenter);
    gridLayout->addWidget(selectedLabel, 0,2,Qt::AlignHCenter);
    gridLayout->addWidget(notSelectedView, 1,0,3,1);
    gridLayout->addWidget(selectedWidget, 1,2,3,1);
    gridLayout->addWidget(addButton,1,1);
    gridLayout->addWidget(removeButton,2,1);
    setLayout(gridLayout);
}
