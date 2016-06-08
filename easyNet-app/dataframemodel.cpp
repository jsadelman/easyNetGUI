#include <QtDebug>
#include <QtGui>
#include <QApplication>
#include "dataframemodel.h"


DataFrameModel::DataFrameModel(QDomDocument *domDoc, QObject *parent)
    :domDoc(domDoc), m_name(), QAbstractTableModel(parent)
{
}

int DataFrameModel::rowCount(const QModelIndex &parent) const
{
    return tBody().childNodes().size() - 1;
}

int DataFrameModel::columnCount(const QModelIndex &parent) const
{
    return tBody().firstChild().childNodes().size() - 1;
}

QVariant DataFrameModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        return tBody().childNodes().at(index.row() + 1).childNodes().at(index.column() +1).toElement().text();
    }
    return QVariant();
}

QVariant DataFrameModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return tBody().firstChild().childNodes().at(section +1).toElement().text();
    }

    else if (orientation == Qt::Vertical && role == Qt::DisplayRole)
        return QString::number(section + 1);
//        return tBody().childNodes().at(section +1).firstChildElement().text();


    return QVariant();
}

QStringList DataFrameModel::rowNames()
{
    QStringList rows;
    for (int i = 0; i < rowCount(); ++i)
        rows.append(headerData(i, Qt::Vertical, Qt::DisplayRole).toString());
    return rows;
}

QStringList DataFrameModel::colNames()
{
    QStringList cols;
    for (int i = 0; i < columnCount(); ++i)
        cols.append(headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
    return cols;
}

QString DataFrameModel::writeTable()
{
    QString output = colNames().join("\t").append("\n");
    QStringList rowList;
    for (int row = 0; row < rowCount(); ++row)
    {
        rowList.clear();
        for (int col = 0; col < columnCount(); ++col)
            rowList.append(data(index(row, col), Qt::DisplayRole).toString());
        output.append(rowList.join("\t").append("\n"));
    }
    return output;
}

Qt::ItemFlags DataFrameModel::flags (const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index) | Qt::ItemIsDragEnabled;
}

DataFrameHeader::DataFrameHeader(QWidget *parent)
    :QHeaderView(Qt::Horizontal, parent)
{
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
}

void DataFrameHeader::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        startPos = event->pos();
        int index = logicalIndexAt( event->pos() );
        text = model()->headerData(index, Qt::Horizontal).toString();
    }
    QHeaderView::mousePressEvent(event );
}

void DataFrameHeader::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        int distance = (event->pos() - startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance())
            performDrag();
    }
    QHeaderView::mouseMoveEvent(event);
}

void DataFrameHeader::performDrag()
{
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(QString("$%1").arg(text));
    drag->setMimeData(mimeData);

    if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
    {
        emit columnDropped(tableName);
    }
    else // may need a condition here to check if there has been a drop already?
        emit restoreComboBoxText();
}

void DataFrameHeader::setTableName(QString name)
{
    tableName = name;
}
