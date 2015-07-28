#ifndef EXPANDTOFILLBUTTON_H
#define EXPANDTOFILLBUTTON_H

#include <QStyledItemDelegate>


class ExpandToFillButton : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ExpandToFillButton(QObject * parent = 0);

    void paint(QPainter * painter, const QStyleOptionViewItem & option,
               const QModelIndex & index) const Q_DECL_OVERRIDE;

signals:
    void expandToFill(QString);

protected:
    bool editorEvent(QEvent * event, QAbstractItemModel * model,
                const QStyleOptionViewItem & option, const QModelIndex & index) Q_DECL_OVERRIDE;
};

#endif // EXPANDTOFILLBUTTON_H
