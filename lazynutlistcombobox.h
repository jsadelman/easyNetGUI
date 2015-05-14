#ifndef LAZYNUTLISTCOMBOBOX_H
#define LAZYNUTLISTCOMBOBOX_H

#include <QComboBox>

class LazyNutListComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit LazyNutListComboBox(QString getListCmd = QString(), QWidget *parent = 0);
    void setEmptyItem(bool present);

signals:
    void listReady();

public slots:
    void setGetListCmd(QString cmd);
    void getList(QString cmd = QString());
    void savePos(const QPoint& pos);

private slots:
    void buildList(QStringList list);

private:
    QString getListCmd;
    int counter;
    QPoint mousePos;

};

#endif // LAZYNUTLISTCOMBOBOX_H
