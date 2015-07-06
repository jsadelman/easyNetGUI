#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QComboBox>

class myComboBox : public QComboBox
{
    Q_OBJECT

public:
    myComboBox(QWidget *parent=0);
    ~myComboBox();

signals:

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
};

#endif // MYCOMBOBOX_H
