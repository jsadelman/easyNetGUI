#ifndef LAZYNUTLISTCOMBOBOX_H
#define LAZYNUTLISTCOMBOBOX_H

#include <QComboBox>

class LazyNutListComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit LazyNutListComboBox(QString getListCmd = QString(), QWidget *parent = 0);

public slots:
    void setGetListCmd(QString cmd);
    void getList(QString cmd = QString());

private slots:
    void buildList(QStringList list);

private:
    QString getListCmd;

};

#endif // LAZYNUTLISTCOMBOBOX_H
