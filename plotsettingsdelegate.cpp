#include "plotsettingsdelegate.h"
#include "plotsettingsmodel.h"
#include "plotsettingsbaseeditor.h"
#include <QDebug>

PlotSettingsDelegate::PlotSettingsDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}


QWidget *PlotSettingsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString type = index.model()->data(index, PlotSettingsModel::PlotSettingsTypeRole).toString();

    if (true)     //(type == "numeric")
    {
       PlotSettingsBaseEditor *editor = new PlotSettingsBaseEditor(parent);
       //setEditorData(editor, index);
       return editor;
    }
}

void PlotSettingsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString name, type, value, comment, defaultValue;
    name =          index.model()->data(index, PlotSettingsModel::PlotSettingsNameRole).toString();
    type =          index.model()->data(index, PlotSettingsModel::PlotSettingsTypeRole).toString();
    value =         index.model()->data(index, Qt::EditRole).toString();
    comment =       index.model()->data(index, PlotSettingsModel::PlotSettingsCommentRole).toString();
    defaultValue =  index.model()->data(index, PlotSettingsModel::PlotSettingsDefaultRole).toString();
    if (true)     //(type == "numeric")
    {
        PlotSettingsBaseEditor *plotSettingsEditor = static_cast<PlotSettingsBaseEditor*>(editor);
        plotSettingsEditor->setName(name);
        plotSettingsEditor->setComment(comment);
        plotSettingsEditor->setValue(value.isEmpty() ? defaultValue : value);
    }
}

void PlotSettingsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QString type = index.model()->data(index, PlotSettingsModel::PlotSettingsTypeRole).toString();
    if (true)     //(type == "numeric")
    {
        PlotSettingsBaseEditor *plotSettingsEditor = static_cast<PlotSettingsBaseEditor*>(editor);
        model->setData(index, plotSettingsEditor->getValue(), Qt::EditRole);
    }
}

void PlotSettingsDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
    qDebug() << option.rect;
}


