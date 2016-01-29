#ifndef CLOSEBUTTON_H
#define CLOSEBUTTON_H

#include <QStyledItemDelegate>
#include <QApplication>

// based on http://stackoverflow.com/questions/7175333/howto-create-delegate-for-qtreewidget

class CloseButton : public QStyledItemDelegate
{
    Q_OBJECT
public:

    explicit CloseButton(QObject *parent = 0,
                         const QPixmap &closeIcon = QPixmap());

    QPoint closeIconPos(const QStyleOptionViewItem &option) const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) Q_DECL_OVERRIDE;

signals:
    void closeIndexClicked(const QModelIndex &);
private:
    QPixmap m_closeIcon;
    static const int margin = 0; // pixels to keep arount the icon

    Q_DISABLE_COPY(CloseButton)
};

#endif // CLOSEBUTTON_H
