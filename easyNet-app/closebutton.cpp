#include "closebutton.h"
#include <QPainter>
#include <QMouseEvent>

CloseButton::CloseButton(QObject *parent, const QPixmap &closeIcon)
    : QStyledItemDelegate(parent)
    , m_closeIcon(closeIcon)
{
    if(m_closeIcon.isNull())
    {
        m_closeIcon = qApp->style()->standardPixmap(QStyle::SP_DockWidgetCloseButton);
    }
}

QPoint CloseButton::closeIconPos(const QStyleOptionViewItem &option) const {
    return QPoint(option.rect.right() - m_closeIcon.width() - margin,
                  option.rect.center().y() - m_closeIcon.height()/2);
}

void CloseButton::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);

    if (option.state & QStyle::State_MouseOver)
    {
        QRect closeButtonRect = m_closeIcon.rect().translated(closeIconPos(option));
        QPen pen = painter->pen();
        painter->setPen(Qt::red);
        painter->drawRect(closeButtonRect);
        painter->setPen(pen);
    }
    painter->drawPixmap(closeIconPos(option), m_closeIcon);
}

QSize CloseButton::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);

    // Make some room for the close icon
    size.rwidth() += m_closeIcon.width() + margin * 2;
    size.setHeight(qMax(size.height(), m_closeIcon.height() + margin * 2));
    return size;
}

bool CloseButton::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // Emit a signal when the icon is clicked
    if(event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QRect closeButtonRect = m_closeIcon.rect().translated(closeIconPos(option));
        if(closeButtonRect.contains(mouseEvent->pos()))
        {
            emit closeIndexClicked(index);
        }
    }
    return false;
}
