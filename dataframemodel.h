#ifndef DATAFRAMEMODEL_H
#define DATAFRAMEMODEL_H

#include <QAbstractTableModel>

#include <QDomDocument>
#include <QHeaderView>
#include <QWidget>

class QMimeData;


class DataFrameModel : public QAbstractTableModel
{
    /* table model for the following XML format:
     *
<TABLE>
    <TBODY>
        <TR>
            <TH></TH>
            <TH>stimulus</TH>
            <TH>Frequency</TH>
            <TH>N</TH>
        </TR>
        <TR>
            <TH>side</TH>
            <string>side</string>
            <string>High</string>
            <string>Large</string>
        </TR>
        <TR>
            <TH>take</TH>
            <string>take</string>
            <string>High</string>
            <string>Large</string>
        </TR>
    </TBODY>
</TABLE>
     *
     * Assumes that tabel headers always exist for rows and cols.
     *
    */
Q_OBJECT
public:
    DataFrameModel(QDomDocument *domDoc, QObject *parent=0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    QStringList rowNames();
    QStringList colNames();
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);

    signals:
        newParamValueSig (QString);

private slots:
private:
    QDomNode tBody() const {return domDoc->firstChild().firstChild();}
    QDomDocument *domDoc;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QString tableName;
};

class DataFrameHeader : public QHeaderView
{
    Q_OBJECT
    void mousePressEvent(QMouseEvent *event);
public:
        DataFrameHeader(QWidget *parent = 0);

public slots:
        void setTableName(QString name);
signals:
        void columnDropped(QString set);
private:
        QString tableName;

};

#endif // DATAFRAMEMODEL_H
