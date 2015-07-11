#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QComboBox>

class myComboBox : public QComboBox
{
    Q_OBJECT

public:
    myComboBox(QWidget *parent=0);
    ~myComboBox();
    QString         savedComboBoxText;
    QString         thisArg;

    void setArg(QString arg);
public slots:
    void restoreComboBoxText();
signals:
    void argWasChanged(QString arg);
protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);

private:

};

#endif // MYCOMBOBOX_H
