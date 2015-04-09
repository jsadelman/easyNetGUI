#ifndef LAZYNUTLISTMENU_H
#define LAZYNUTLISTMENU_H

#include <QMenu>

class LazyNutListMenu : public QMenu
{
    Q_OBJECT
public:
    explicit LazyNutListMenu(QWidget *parent = 0);

signals:
    void selected(QString);

public slots:
    void setGetListCmd(QString cmd);

private slots:
    void getList();
    void textFromAction(QAction* action);
    void buildMenu(QStringList list);

private:
    QString getListCmd;

};

#endif // LAZYNUTLISTMENU_H
