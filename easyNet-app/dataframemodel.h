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
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    DataFrameModel(QDomDocument *domDoc, QObject *parent=0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    QStringList rowNames();
    QStringList colNames();
    QString name() {return m_name;}
    QString writeTable();

public slots:
    void setName(QString name) {m_name = name;}

protected:
    QDomNode tBody() const {return domDoc->firstChild().firstChild();}
    QDomDocument *domDoc;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QString m_name;
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
        void restoreComboBoxText();
protected:
        void mouseMoveEvent(QMouseEvent *event);
private:
        QString tableName;
        QPoint startPos;
        QString text;

        void performDrag();
};

#endif // DATAFRAMEMODEL_H
