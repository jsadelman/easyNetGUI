#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QDockWidget>

class QListView;
class QButton;
class QAbstractItemView;
class QToolBar;
class CheckListModel;

class HistoryWidget : public QDockWidget
{
    Q_OBJECT
public:
    HistoryWidget(QWidget* parent = 0);
    QListView* view() {return m_view;}
    void setModel(CheckListModel *model);
    QAction *moveToViewerAct;
    QAction *destroyAct;

signals:
    void clicked(QString);

private slots:
    void selectAll();
    void clearSelection();


private:
    void buildWidget();
    void createActions();
    void createWidgets();
    void checkAll(int check);


    QListView *m_view;
    QAction *selectAllAct;
    QAction *clearSelectionAct;

    QToolBar *toolBar;
    bool m_checkDataChanged;
};

#endif // HISTORYWIDGET_H
