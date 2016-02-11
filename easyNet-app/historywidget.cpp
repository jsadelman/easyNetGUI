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
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    view = new QTreeView;
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setHeaderHidden(true);
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
