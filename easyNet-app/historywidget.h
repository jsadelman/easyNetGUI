#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QDockWidget>

class QTreeView;
class QAbstractItemView;
class QToolBar;
class HistoryTreeModel;

class HistoryWidget : public QDockWidget
{
    Q_OBJECT
    friend class DataViewerDispatcher;
public:
    HistoryWidget(QWidget* parent = 0);
    void setModel(HistoryTreeModel *model);

signals:
    void clicked(QString);

private:
    void buildWidget();
    QTreeView *view;
    QAction *selectAllAct;
    QAction *clearSelectionAct;
};

#endif // HISTORYWIDGET_H
