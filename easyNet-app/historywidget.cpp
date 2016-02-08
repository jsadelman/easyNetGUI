#include "historywidget.h"
#include "historytreemodel.h"

#include <QAction>
#include <QVBoxLayout>
#include <QToolBar>
#include <QDebug>
#include <QTreeView>

HistoryWidget::HistoryWidget(QWidget *parent)
    : QDockWidget(parent)
{
    buildWidget();
}

void HistoryWidget::setModel(HistoryTreeModel *model)
{
    view->setModel(model);
}

void HistoryWidget::buildWidget()
{
    createActions();
    createWidgets();
}

void HistoryWidget::createActions()
{
//    selectAllAct = new QAction(QIcon(":/images/select_all.png"), "select all", this);
//    selectAllAct->setToolTip("select all items");
//    connect(selectAllAct, SIGNAL(triggered()), this, SLOT(selectAll()));

//    clearSelectionAct = new QAction(QIcon(":/images/clear_selection.png"), "clear all", this);
//    clearSelectionAct->setToolTip("clear selection");
//    connect(clearSelectionAct, SIGNAL(triggered()), this, SLOT(clearSelection()));

    destroyAct = new QAction(QIcon(":/images/icon_trash.png"), "delete", this);
    destroyAct->setToolTip("delete selected items");
}

void HistoryWidget::createWidgets()
{
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    toolBar = new QToolBar;
    toolBar->addActions(QList<QAction*>{destroyAct});
    layout->addWidget(toolBar);
    view = new QTreeView;
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layout->addWidget(view);
    mainWidget->setLayout(layout);
    setWidget(mainWidget);
    setWindowTitle("History");

    connect(view, &QTreeView::clicked, [=](const QModelIndex & index)
    {
        QString item = view->model()->data(index).toString();
        emit clicked(item);
    });

}

//void HistoryWidget::checkAll(int check)
//{
////    HistoryModel* historyModel = static_cast<HistoryModel*>(m_view->model());
//    for (int row = 0; row < m_view->model()->rowCount(); ++ row)
//        m_view->model()->setData(m_view->model()->index(row, 0), check, Qt::CheckStateRole);
//}

