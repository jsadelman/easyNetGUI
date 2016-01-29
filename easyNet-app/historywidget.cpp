#include "historywidget.h"
#include "historymodel.h"

#include <QAction>
#include <QVBoxLayout>
#include <QToolBar>
#include <QDebug>
#include <QListView>

HistoryWidget::HistoryWidget(QWidget *parent)
    : QDockWidget(parent), m_checkDataChanged(false)
{
    buildWidget();
}

void HistoryWidget::setModel(CheckListModel *model)
{
    m_view->setModel(model);
    connect(model, &CheckListModel::checkDataChanged, [=]()
    {
        m_checkDataChanged = true;
    });
}

void HistoryWidget::selectAll()
{
    qDebug() << Q_FUNC_INFO;
    checkAll(Qt::Checked);

}

void HistoryWidget::clearSelection()
{
    qDebug() << Q_FUNC_INFO;
    checkAll(Qt::Unchecked);

}



void HistoryWidget::buildWidget()
{
    createActions();
    createWidgets();
}

void HistoryWidget::createActions()
{
    selectAllAct = new QAction(QIcon(":/images/select_all.png"), "select all", this);
    selectAllAct->setToolTip("select all items");
    connect(selectAllAct, SIGNAL(triggered()), this, SLOT(selectAll()));

    clearSelectionAct = new QAction(QIcon(":/images/clear_selection.png"), "clear all", this);
    clearSelectionAct->setToolTip("clear selection");
    connect(clearSelectionAct, SIGNAL(triggered()), this, SLOT(clearSelection()));

    moveToViewerAct = new QAction(QIcon(":/images/move_to.png"), "to viewer", this);
    moveToViewerAct->setToolTip("move selected items to viewer");

    destroyAct = new QAction(QIcon(":/images/icon_trash.png"), "delete", this);
    destroyAct->setToolTip("delete selected items");


}

void HistoryWidget::createWidgets()
{
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    toolBar = new QToolBar;
    toolBar->addActions(QList<QAction*>{selectAllAct, clearSelectionAct, moveToViewerAct, destroyAct});
    layout->addWidget(toolBar);
    m_view = new QListView;
    layout->addWidget(m_view);
    mainWidget->setLayout(layout);
    setWidget(mainWidget);
    setWindowTitle("History");

    connect(m_view, &QListView::clicked, [=](const QModelIndex & index)
    {
        if (!m_checkDataChanged)
        {
            QString item = m_view->model()->data(index).toString();
            emit clicked(item);
        }
        m_checkDataChanged = false;
    });
}

void HistoryWidget::checkAll(int check)
{
//    HistoryModel* historyModel = static_cast<HistoryModel*>(m_view->model());
    for (int row = 0; row < m_view->model()->rowCount(); ++ row)
        m_view->model()->setData(m_view->model()->index(row, 0), check, Qt::CheckStateRole);
}

