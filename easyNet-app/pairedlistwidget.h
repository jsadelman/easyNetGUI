#ifndef PAIREDLISTWIDGET_H
#define PAIREDLISTWIDGET_H

#include <QFrame>

class HideFromListModel;
class QAbstractItemModel;
class QListView;
class QListWidget;
class QPushButton;
class QLabel;
class QLineEdit;

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
    void buildWidget();

    QAbstractItemModel *listModel;
    int relevantColumn;
    HideFromListModel *notSelectedModel;
    QListView *notSelectedView;
    QListWidget *selectedWidget;
    QLineEdit *searchEdit;
    QPushButton *addButton;
    QPushButton *removeButton;
    QLabel *selectedLabel;
    QLabel *notSelectedLabel;

};

#endif // PAIREDLISTWIDGET_H
