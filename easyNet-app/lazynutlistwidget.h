#ifndef LAZYNUTLISTWIDGET_H
#define LAZYNUTLISTWIDGET_H

#include <QListWidget>

class LazyNutListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit LazyNutListWidget(QString getListCmd, QWidget *parent = 0);

public slots:
    void setGetListCmd(QString cmd);
    void getList(QString cmd = QString());
    QString currentValue();
    void setCurrentValue(QString value);

signals:
    void selected(QString);

private slots:
    void buildList(QStringList list);
    void item2text(QListWidgetItem * item);

private:
    QString getListCmd;

};

#endif // LAZYNUTLISTWIDGET_H
