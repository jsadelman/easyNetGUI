#include <QPainter>
#include <QEvent>
#include <QDebug>

#include "expandtofillbutton.h"
#include "enumclasses.h"

ExpandToFillButton::ExpandToFillButton(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ExpandToFillButton::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(ExpandToFillRole).toBool() == true)
    {
        painter->setPen(QColor("blue"));
        painter->drawText(option.rect, "<Click to view>");
    }
    else
        QStyledItemDelegate::paint(painter, option, index);
}

bool ExpandToFillButton::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.data(ExpandToFillRole).toBool() == true && event->type() == QEvent::MouseButtonRelease)
    {
         emit expandToFill(model,index,index.data(Qt::DisplayRole).toString());
    }
    return false;
}

