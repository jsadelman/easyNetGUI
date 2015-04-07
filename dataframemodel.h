#ifndef DATAFRAMEMODEL_H
#define DATAFRAMEMODEL_H

#include <QAbstractTableModel>

#include <QDomDocument>

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

private:
    QDomNode tBody() const {return domDoc->firstChild().firstChild();}
    QDomDocument *domDoc;
};

#endif // DATAFRAMEMODEL_H
