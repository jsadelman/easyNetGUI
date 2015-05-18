#ifndef LAZYNUTLISTCOMBOBOX_H
#define LAZYNUTLISTCOMBOBOX_H

#include <QComboBox>
#include <QPoint>
#include <Qt>
#include <QEvent>
#include <QMutex>

struct EventParams
{
    QEvent::Type            type;
    QPoint                  localPos;
    Qt::MouseButton         button;
    Qt::MouseButtons        buttons;
    Qt::KeyboardModifiers   modifiers;

    void clear()
    {
        type = QEvent::None;
        localPos = QPoint();
        button = Qt::NoButton;
        buttons = Qt::NoButton;
        modifiers = Qt::NoModifier;
    }
};

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

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void buildList(QStringList list);
    void __debug_getList(QString = QString());

private:
    void repostEvent();

    QString getListCmd;
    bool eventSwitch;
    QMutex eventMutex;
    EventParams eventParams;
    QString savedCurrentText;
    int __debug_getList_counter = 0;

};

#endif // LAZYNUTLISTCOMBOBOX_H
