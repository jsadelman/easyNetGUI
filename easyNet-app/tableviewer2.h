#ifndef TABLEVIEWER2_H
#define TABLEVIEWER2_H

#include <QDialog>
#include <QMainWindow>
#include <QTextDocument>

QT_BEGIN_NAMESPACE
class QToolBar;
class QDialogButtonBox;
class QPushButton;
class QSqlTableModel;
class QDomDocument;
class QLabel;
class QTableView;
class QVBoxLayout;
class QHBoxLayout;
class QListView;
class QModelIndex;
class QStandardItemModel;
class QMimeData;
class QComboBox;

//class ObjectCatalogue;
class DataFrameModel;
class DataFrameHeader;
//class ObjectCatalogueFilter;
class FindDialog;

QT_END_NAMESPACE


class TableViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit TableViewer(const QString &tableName, QWidget *parent = 0);
    QStringList mimeTypes() const;
    QMimeData *mimeData; // (const QModelIndexList &indexes) const;

public slots:
    void setTableText(QString text);
    int addTable(QString name);
signals:
    void currentKeyChanged(QString key);
    void newTableSelection(QString name);
    void setParamDataFrameSignal(QString);
    void newParamValueSig(QString);
    void columnDropped(QString set);
    void restoreComboBoxText();
    void newTableName(QString);
    void openFileRequest();

private slots:
//    void submit();
    void addDataFrameToWidget(QDomDocument* domDoc);
    void rowChangedSlot( const QModelIndex& selected, const QModelIndex& deselected );
    void updateParamTable(QString model);
    void resizeColumns();
    void setView(QString name);

    void on_copy_clicked();
    void updateTableView(QString text);
    void showFindDialog();
    void findForward(const QString &str, QFlags<QTextDocument::FindFlag> flags);
    void save();
    void on_copy_DF_clicked();
private:

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *copyAct;
    QAction *copyDFAct;
    QAction *findAct;

    QPushButton *submitButton;
    QPushButton *revertButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
//    QSqlTableModel *model;
    QStandardItemModel *model;
    DataFrameModel  *dfModel;
    QTableView      *view;
    QComboBox       *tableBox;
    QLabel          *listTitle;
    QStringList     list;
    QWidget         *widget;
    QString         currentTable;
    FindDialog*     findDialog;

    QTabWidget*     tablePanel;
    QVector <QTableView*>  tables;
    int             currentTableIdx;
    int             numTables;
    QMap <int, QString> tableMap;
    QVector <DataFrameHeader*> myHeaders;


    void createToolBars();
    void createActions();
    void setViewToStringList();
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void mousePressEvent(QMouseEvent *event);

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    void replaceHeaders(QTableView *view);
};


#endif
