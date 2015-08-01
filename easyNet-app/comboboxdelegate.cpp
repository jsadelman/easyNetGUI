#include "comboboxdelegate.h"
#include <QComboBox>
#include <QApplication>
#include <QDebug>
#include <lazynutobj.h>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include "lazynutobj.h"

ComboBoxInDelegate::ComboBoxInDelegate(const QModelIndex &delegateModelIndex, QWidget *parent):
    QComboBox(parent),delegateModelIndex(delegateModelIndex)
{
    connect(this, SIGNAL(activated(int)), this, SLOT(associateDelegateIndex(int)));
    delegateRow = delegateModelIndex.row();
}

void ComboBoxInDelegate::associateDelegateIndex(int objIndex)
{
     emit activatedDelegateIndex(delegateModelIndex,objIndex);
}




QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
    const QModelIndex & index) const
{
    ComboBoxInDelegate *editor = new ComboBoxInDelegate(index,parent);
    editor->addItems(index.model()->data(index, Qt::DisplayRole).toStringList());
    QBrush brush;
    brush.setColor(Qt::blue);
    for (int i = 0; i < editor->count(); ++i)
    {
        if (_objHash->contains(editor->itemData(i,Qt::DisplayRole).toString()))
            editor->setItemData(i,brush,Qt::ForegroundRole);
    }
    connect(editor,SIGNAL(activatedDelegateIndex(const QModelIndex&,int)),
            this,SIGNAL(activatedDelegateIndex(const QModelIndex&,int)));
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &/*index*/) const
 {
     //QStringList values = index.model()->data(index, Qt::EditRole).toStringList();

     QComboBox *comboBox = static_cast<QComboBox*>(editor);
     comboBox->setCurrentIndex(0);
     //comboBox->addItems(index.model()->data(index, Qt::DisplayRole).toStringList());
 }

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex & ) const
 {
     editor->setGeometry(option.rect);
 }

void ComboBoxDelegate::setModelData(QWidget *, QAbstractItemModel *,
                    const QModelIndex &) const
{}


/*void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyleOptionViewItemV4 myOption = option;

  //QString text = index.model()->data(index, Qt::EditRole).toStringList().at(0);
  myOption.text = "paint";

  QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}
*/
