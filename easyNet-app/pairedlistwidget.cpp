#include "pairedlistwidget.h"

#include "selectfromlistmodel.h"
#include <QSortFilterProxyModel>
#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QDebug>

PairedListWidget::PairedListWidget(QAbstractItemModel *listModel, int relevantColumn, QWidget *parent)
    : listModel(listModel), relevantColumn(relevantColumn), QFrame(parent)
{
    if (listModel)
    {
        setupModels();
        buildWidget();
    }
}

void PairedListWidget::setValue(QStringList selectedList)
{
    selectedView->selectAll();
    moveSelected(selectedView, false);
    foreach (QString val, selectedList)
    {
        QModelIndexList matchList = selectFromListModel->match(
                    selectFromListModel->index(0,0),
                    Qt::DisplayRole,
                    val,
                    1,
                    Qt::MatchExactly
                    );
        if (!matchList.isEmpty())
        {
            QModelIndex idx = selectFromListModel->index(matchList.first().row(),1);
            selectFromListModel->setData(idx,true);
        }
    }
    emit valueChanged();
}

QStringList PairedListWidget::getValue()
{
    QStringList list;
    for (int row = 0; row < selectedModel->rowCount(); ++row)
        list.append(selectedModel->data(selectedModel->index(row, 0)).toString());
    return list;
}

void PairedListWidget::addSelected()
{
    if (moveSelected(notSelectedView, true))
        emit valueChanged();
}

void PairedListWidget::removeSelected()
{
    if (moveSelected(selectedView, false))
        emit valueChanged();
}

bool PairedListWidget::moveSelected(QListView *fromListView, bool selected)
{
    if (fromListView->selectionModel()->selectedIndexes().isEmpty())
        return false;
    while (!fromListView->selectionModel()->selectedIndexes().isEmpty())
    {
        selectFromListModel->setData(
                    selectFromListModel->index(
                        qobject_cast<QSortFilterProxyModel*>(fromListView->model())->mapToSource(
                                fromListView->selectionModel()->selectedIndexes().first()
                            ).row(),1),
                    selected);
    }
    return true;
}

void PairedListWidget::setupModels()
{
    selectFromListModel = new SelectFromListModel(this);
    selectFromListModel->setSourceModel(listModel);
    selectFromListModel->setRelevantColumn(relevantColumn);
    selectedModel = new QSortFilterProxyModel(this);
    selectedModel->setSourceModel(selectFromListModel);
    selectedModel->setFilterRegExp("true");
    selectedModel->setFilterKeyColumn(1);
    notSelectedModel = new QSortFilterProxyModel(this);
    notSelectedModel->setSourceModel(selectFromListModel);
    notSelectedModel->setFilterRegExp("false");
    notSelectedModel->setFilterKeyColumn(1);
    notSelectedModel->sort(0);
}

void PairedListWidget::buildWidget()
{
    QGridLayout *gridLayout = new QGridLayout;
    selectedView = new QListView(this);
    selectedView->setModel(selectedModel);
    selectedView->setModelColumn(0);
    selectedView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    notSelectedView = new QListView(this);
    notSelectedView->setModel(notSelectedModel);
    notSelectedView->setModelColumn(0);
    notSelectedView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    addButton = new QPushButton("==>", this);
    addButton->setToolTip("Add to selected factors");
    connect(addButton, SIGNAL(clicked()), this, SLOT(addSelected()));
    removeButton = new QPushButton("<==", this);
    removeButton->setToolTip("Remove from selected factors");
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
    notSelectedLabel = new QLabel("Available factors");
    selectedLabel = new QLabel("Selected factors");
    gridLayout->addWidget(notSelectedLabel, 0,0,Qt::AlignHCenter);
    gridLayout->addWidget(selectedLabel, 0,2,Qt::AlignHCenter);
    gridLayout->addWidget(notSelectedView, 1,0,3,1);
    gridLayout->addWidget(selectedView, 1,2,3,1);
    gridLayout->addWidget(addButton,1,1);
    gridLayout->addWidget(removeButton,2,1);
    setLayout(gridLayout);
}
