#include "historywidget.h"
#include "historytreemodel.h"
#include "enumclasses.h"

#include <QAction>
#include <QVBoxLayout>
#include <QToolBar>
#include <QDebug>
#include <QTreeView>

HistoryWidget::HistoryWidget(QWidget *parent, QString windowTitle)
    : QDockWidget(parent), windowTitle(windowTitle)
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
    setWindowTitle(windowTitle);

    connect(view, &QTreeView::clicked, [=](const QModelIndex & index)
    {
        QString item = view->model()->data(index, NameRole).toString();
        emit clicked(item);
    });

}
