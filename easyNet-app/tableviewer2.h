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
class QAbstractItemModel;
class QIdentityProxyModel;

//class ObjectCache;
class DataFrameModel;
class DataFrameHeader;
//class ObjectCatalogueFilter;
class FindDialog;
class TrialDataFrameModel;

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
    QString addTable(QString name="", bool chooseNewName=true);
    void addTrialTable(QString name);
    void switchTab(QString tableName);
    void addTableWithThisName(QString name);
    void addDataFrameToWidget(QDomDocument* domDoc, QString cmd);
    void prepareToAddDataFrameToWidget(QDomDocument* domDoc, QString cmd);

signals:
    void currentKeyChanged(QString key);
    void newTableSelection(QString name);
    void setParamDataFrameSignal(QString);
    void newParamValueSig(QString);
    void columnDropped(QString set);
    void restoreComboBoxText();
    void newTableName(QString);
    void openFileRequest();
    void createNewPlotOfType(QString, QString, QMap<QString, QString>);
    void showPlotSettings();

private slots:
//    void submit();
    void rowChangedSlot( const QModelIndex& selected, const QModelIndex& deselected );
    void updateParamTable(QString model);
    void resizeColumns();
    void setView(QString name);
    void setPrettyHeaderFromJob();

    void on_copy_clicked();
    void updateTableView(QString text);
    void showFindDialog();
    void findForward(const QString &str, QFlags<QTextDocument::FindFlag> flags);
    void save();
    void on_copy_DF_clicked();
    void preparePlot();
    void mergeFD();
    void refresh();
private:

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *copyAct;
    QAction *copyDFAct;
    QAction *mergeDFAct;
    QAction *findAct;
    QAction *plotAct;
    QAction *refreshAct;

    QPushButton *submitButton;
    QPushButton *revertButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
//    QSqlTableModel *model;
    QStandardItemModel *model;
    DataFrameModel  *lastAddedModel;
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
    int             lastAddedDataFrameIdx;
    QMap <int, QString> tableMap;
    QVector <DataFrameHeader*> myHeaders;


    void createToolBars();
    void createActions();
    void setViewToStringList();
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void mousePressEvent(QMouseEvent *event);

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    void setModelAtTableIdx(QAbstractItemModel *model, int idx);
};


#endif
