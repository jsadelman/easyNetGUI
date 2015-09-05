#ifndef TABLEEDITOR_H
#define TABLEEDITOR_H

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

class ObjectCatalogue;
class DataFrameModel;
class ObjectCatalogueFilter;
class FindDialog;

QT_END_NAMESPACE


class TableEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit TableEditor(ObjectCatalogue *objectCatalogue, const QString &tableName, QWidget *parent = 0);
    explicit TableEditor(const QString &tableName, QWidget *parent = 0);
    void setFilter(QString type);
    QStringList mimeTypes() const;
    QMimeData *mimeData; // (const QModelIndexList &indexes) const;

public slots:
    void setTableText(QString text);
    void selectTable(QString text);
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

    void refresh();
private:

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *copyAct;
    QAction *copyDFAct;
    QAction *findAct;
    QAction *refreshAct;

    ObjectCatalogue  *objectCatalogue;
    ObjectCatalogueFilter *objectListFilter;
    QListView *objectListView;

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

    void createToolBars();
    void createActions();
    void init(const QString &tableName, QWidget *parent);
    void setViewToStringList();
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void mousePressEvent(QMouseEvent *event);

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
};


#endif
