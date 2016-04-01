#include <QtDebug>
#include <QtGui>
#include <QApplication>
#include "dataframemodel.h"


DataFrameModel::DataFrameModel(QDomDocument *domDoc, QObject *parent)
    :domDoc(domDoc), m_name(), QAbstractTableModel(parent)
{
//    qDebug() << domDoc->toString();
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
        return tBody().childNodes().at(section +1).firstChildElement().text();

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

bool DataFrameModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.column() == 0) // can't edit parameter names
            return false;
    // check legality
    bool ok;
    float fvalue = value.toFloat(&ok);
    if (!ok) // not a float
        return false;
    // NOTE: hack for parameters dataframe only, would not work e.g. for editing a stimulus set
    if (role == Qt::EditRole)
    {
        tBody().childNodes().at(index.row() + 1).childNodes().at(index.column() +1).firstChild().setNodeValue(value.toString());
        QString newParamValue = tBody().childNodes().at(index.row() + 1).childNodes().at(index.column()).toElement().text() +
                " " + value.toString();
        emit newParamValueSig(m_name,newParamValue);

    }
    return true;
}


Qt::ItemFlags DataFrameModel::flags (const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
    // same criteria as in setData
    if (index.column() == 0)
        return defaultFlags;

    return Qt::ItemIsDragEnabled | Qt::ItemIsEditable  | defaultFlags;

}

//QStringList DataFrameModel::mimeTypes() const
//{
//    QStringList types;
//    types << "application/vnd.text.list";
//    return types;
//}

//QMimeData* DataFrameModel::mimeData(const QModelIndexList &indexes) const
//{
//    QMimeData *mimeData = new QMimeData();
//    QByteArray encodedData;

//    QDataStream stream(&encodedData, QIODevice::WriteOnly);

//    foreach (const QModelIndex &index, indexes) {
//        if (index.isValid()) {
//            QString text = data(index, Qt::DisplayRole).toString();
//            stream << text;
//        }
//    }

//    mimeData->setData("application/vnd.text.list", encodedData);
//    return mimeData;
//}


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


/*
    //QByteArray itemData;
    //QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    //dataStream << QPoint(event->pos() - child->pos());
    int index;
    QString text;

    if (event->button() == Qt::LeftButton)
//        && iconLabel->geometry().contains(event->pos()))
    {
        index = logicalIndexAt( event->pos() );
        text = model()->headerData(index, Qt::Horizontal).toString();
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(QString("$")+text);
        drag->setMimeData(mimeData);
        Qt::DropAction dropAction = drag->exec();
        qDebug() << "Initiating drag from headerview" << text;
        qDebug() << "return from drag->exec is" << dropAction;
        if (dropAction != Qt::IgnoreAction)
        {
            qDebug() << "Emitting column dropped, table name = " << tableName;
            emit columnDropped(tableName);
        }

    }
    else
        QHeaderView::mousePressEvent(event );
*/
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
//    qDebug() << "performDrag, text = " << text;
    mimeData->setText(QString("$%1").arg(text));

    drag->setMimeData(mimeData);
//    qDebug() << "performDrag, mimeData = " << mimeData;

    if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
    {
//        qDebug() << "moved header";
        emit columnDropped(tableName);
    }
    else // may need a condition here to check if there has been a drop already?
        emit restoreComboBoxText();
}

void DataFrameHeader::setTableName(QString name)
{
    tableName = name;
    qDebug() << "DataFrameHeader: tableName set to " << tableName;
}
