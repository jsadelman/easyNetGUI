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
//    QTreeView* view() {return m_view;}
    void setModel(HistoryTreeModel *model);
//    QAction *moveToViewerAct;
    QAction *destroyAct;

signals:
    void clicked(QString);

private slots:
//    void selectAll();
//    void clearSelection();


private:
    void buildWidget();
    void createActions();
    void createWidgets();
//    void checkAll(int check);


    QTreeView *view;
    QAction *selectAllAct;
    QAction *clearSelectionAct;

    QToolBar *toolBar;
//    bool m_checkDataChanged;
};

#endif // HISTORYWIDGET_H
