#ifndef LAZYNUTPAIREDLISTWIDGET_H
#define LAZYNUTPAIREDLISTWIDGET_H

#include <QMainWindow>

class QPushButton;
class QAction;
class QToolBar;
class QFrame;
class QListWidget;
class QListWidgetItem;


class LazyNutPairedListWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit LazyNutPairedListWidget(QString getListCmd = QString(), QWidget *parent = 0);

signals:
    void listReady();
    void valueChanged();


public slots:
    void setGetListCmd(QString cmd);
    void getList(QString cmd = QString());
    void setValue(QStringList list);
    QStringList getValue();


private slots:
    void buildList(QStringList list);
    void showHide();
    void addItems();
    void removeItems();


private:
    void moveItems(QListWidget *fromList, QListWidget *toList, QList<QListWidgetItem *> items);

    QString getListCmd;
    QPushButton *showHideButton;
    QAction *showHideAct;
    QFrame *editFrame;
    QListWidget *factorList;
    QListWidget *selectedList;
    int frameHeight;
};

#endif // LAZYNUTPAIREDLISTWIDGET_H
