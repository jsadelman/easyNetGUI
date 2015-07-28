#ifndef PAIREDLISTWIDGET_H
#define PAIREDLISTWIDGET_H

#include <QFrame>

class SelectFromListModel;
class QSortFilterProxyModel;
class QAbstractItemModel;
class QListView;
class QPushButton;
class QLabel;

class PairedListWidget : public QFrame
{
    Q_OBJECT
public:
    friend class MainWindow;
    explicit PairedListWidget(QAbstractItemModel *listModel, int relevantColumn = 0, QWidget *parent = 0);

signals:
    void valueChanged();

public slots:
    void setValue(QStringList selectedList);
    QStringList getValue();


private slots:
    void addSelected();
    void removeSelected();



private:
    bool moveSelected(QListView *fromListView, bool selected);
    void setupModels();
    void buildWidget();

    QAbstractItemModel *listModel;
    int relevantColumn;
    SelectFromListModel *selectFromListModel;
    QSortFilterProxyModel *selectedModel;
    QSortFilterProxyModel *notSelectedModel;
    QListView *selectedView;
    QListView *notSelectedView;
    QPushButton *addButton;
    QPushButton *removeButton;
    QLabel *selectedLabel;
    QLabel *notSelectedLabel;

};

#endif // PAIREDLISTWIDGET_H
