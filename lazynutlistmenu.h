#ifndef LAZYNUTLISTMENU_H
#define LAZYNUTLISTMENU_H

#include <QMenu>

class LazyNutListMenu : public QMenu
{
    Q_OBJECT
public:
    explicit LazyNutListMenu(QWidget *parent = 0);
    void prePopulate(QString text);
    void prePopulate(QStringList list);

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
    QStringList preItems;

};

#endif // LAZYNUTLISTMENU_H
