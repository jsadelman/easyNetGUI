#include <QDomDocument>
#include <QtWidgets>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QtSql>
#include <QStandardItemModel>


#include "tableeditor.h"
#include "dataframemodel.h"
#include "objectcatalogue.h"
#include "objectcataloguefilter.h"

TableEditor::TableEditor(const QString &tableName, QWidget *parent)
    : QMainWindow(parent)
{
    // this style of constructor is used for stimSetForm
    init(tableName, parent);
//    listTitle->hide();
    objectListView->hide();
    connect(this,SIGNAL(currentKeyChanged(QString)),
            parent,SLOT(currentStimulusChanged(QString)));
    // can't edit stimulus sets (at present)
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // but can edit parameter sets
    if (tableName=="Parameters")
        view->setEditTriggers(QAbstractItemView::AllEditTriggers);

    setViewToStringList(); // temp!!!
}

TableEditor::TableEditor(ObjectCatalogue *_objectCatalogue, const QString &tableName, QWidget *parent)
    : QMainWindow (parent)
{
    // this style of constructor is used for dataframes in the Tables tab
    init(tableName, parent);
    objectCatalogue = _objectCatalogue;
    setFilter("dataframe");
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

}

TableEditor::init(const QString &tableName, QWidget *parent)
{
    createActions();
    createToolBars();

    view = new QTableView(this);
    objectListView = new QListView(this);
//    objectListView->setMaximumWidth(200);
//    listTitle = new QLabel("Sets");
//    tableTitle = new QLabel("Title");

//    QVBoxLayout* layout1 = new QVBoxLayout;
//    layout1->addWidget(listTitle);
//    layout1->addWidget(objectListView);

//    QVBoxLayout* layout2 = new QVBoxLayout;
//    layout2->addWidget(tableTitle);
//    layout2->addWidget(view);

//    QHBoxLayout* layout = new QHBoxLayout;
//    layout->addLayout(layout1);
//    layout->addLayout(layout2);
//    setLayout(layout);

    QSplitter *splitter = new QSplitter;
    splitter->setOrientation(Qt::Horizontal);
    splitter->addWidget(objectListView);
    splitter->addWidget(view);
    setCentralWidget(splitter);



    // other appearance stuff here


/*
    model = new QSqlTableModel(this);
    model->setTable(tableName);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("First name"));
    model->setHeaderData(2, Qt::Horizontal, tr("Last name"));

    QTableView *view = new QTableView;
    view->setModel(model);
    view->resizeColumnsToContents();
    submitButton = new QPushButton(tr("Submit"));
    submitButton->setDefault(true);
    revertButton = new QPushButton(tr("&Revert"));
    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox(Qt::Vertical);
    buttonBox->addButton(submitButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(revertButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);
    connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));
    connect(revertButton, SIGNAL(clicked()), model, SLOT(revertAll()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(view);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Cached Table"));
*/
}

void TableEditor::setFilter(QString type)
{
    objectListFilter = new ObjectCatalogueFilter(this);
    objectListFilter->setType(type);

    objectListView->setModel(objectListFilter);
    objectListView->setModelColumn(0);
    objectListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    objectListView->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(objectListView, &QListView::clicked, [=](const QModelIndex & index)
    {
        setView(objectListFilter->data(index).toString());
    });


}

void TableEditor::createActions()
{
    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
//    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));


    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
//    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

//     saveAsAct = new QAction(tr("Save &As..."), this);
//    saveAsAct->setShortcuts(QKeySequence::SaveAs);
//    saveAsAct->setStatusTip(tr("Save the document under a new name"));
//    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));


//    if (!isReadOnly)
//    {
//    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
//    cutAct->setShortcuts(QKeySequence::Cut);
////    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
////                            "clipboard"));
//    connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));
//    cutAct->setEnabled(false);
//    connect(textEdit, SIGNAL(copyAvailable(bool)),
//            cutAct, SLOT(setEnabled(bool)));

//    }

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(on_copy_clicked()));
    copyAct->setEnabled(true);


//    if (!isReadOnly)
//    {
//    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
//    pasteAct->setShortcuts(QKeySequence::Paste);
////    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
////                              "selection"));
//    connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));
//    }

/*    findAct = new QAction(QIcon(":/images/find.png"), tr("&Find"), this);
    copyAct->setShortcuts(QKeySequence::Find);
//    copyAct->setStatusTip(tr("Find text in this window"));
    connect(findAct, SIGNAL(triggered()), textEdit, SLOT(find()));
    findAct->setEnabled(false);
*/


}


void TableEditor::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    if (openAct)
        fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
//    fileToolBar->addAction(saveAsAct);

    editToolBar = addToolBar(tr("Edit"));
//    if (!isReadOnly)
//        editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
//    if (!isReadOnly)
//        editToolBar->addAction(pasteAct);

    fileToolBar->setMovable(false);
    editToolBar->setMovable(false);

}


//void TableEditor::submit()
//{
//    model->database().transaction();
//    if (model->submitAll()) {
//        model->database().commit();
//    } else {
//        model->database().rollback();
//        QMessageBox::warning(this, tr("Cached Table"),
//                             tr("The database reported an error: %1")
//                             .arg(model->lastError().text()));
//    }
//}

void TableEditor::setView(QString name)
{
//    tableTitle->setText(name);
    emit newTableSelection(name);
}

void TableEditor::addDataFrameToWidget(QDomDocument* domDoc)
{
    dfModel = new DataFrameModel(domDoc, this); // you only need this line to load in the entire XML table
    connect(dfModel, SIGNAL(newParamValueSig(QString)),
            this,SIGNAL(newParamValueSig(QString)));

    view->setModel(dfModel);
    view->resizeColumnsToContents();
    view->show();
    // at this point we have a view widget showing the table
    view->verticalHeader()->hide(); // hideColumn(0); // 1st column contains rownames, which user doesn't need

    QItemSelectionModel* selModel = view->selectionModel();
    connect(selModel, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this,SLOT(rowChangedSlot( const QModelIndex& , const QModelIndex& )));
}

void TableEditor::setViewToStringList()
{
    model = new QStandardItemModel();
//    list = new QStringList;

    model->setHorizontalHeaderLabels({"Index","Command","Time"});

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);

    proxyModel->setSourceModel(model);

    view->setModel(proxyModel);
//    view->setModel(model);
    view->resizeColumnsToContents();
    view->verticalHeader()->hide(); // hideColumn(0); // 1st column contains rownames, which user doesn't need
    view->show();
    view->setSortingEnabled(true);
}

void TableEditor::addRowToTable(QString text)
{
    QList<QStandardItem *> rowData;
//    QStandardItem *idx;
//    idx = new QStandardItem;
    int i = model->rowCount(); // + 1;
    QString number = QString("%1").arg(i, 4, 10, QChar('0'));
    rowData << new QStandardItem(number);
//    idx->setData(i, Qt::DisplayRole);
//    rowData << idx;
    rowData << new QStandardItem(text);
    double r = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
    rowData << new QStandardItem(QString::number(r)); // this will contain the time
    model->appendRow(rowData);
//    model->setItem(i,0,idx);
    view->resizeColumnsToContents();
}

void TableEditor::rowChangedSlot( const QModelIndex& selected, const QModelIndex& deselected )
{
    QString key = (view->model()->data(selected.sibling(selected.row(), 0) ,0)).toString();
//    qDebug() << key;
    emit currentKeyChanged(key);

}

void TableEditor::updateParamTable(QString model)
{
//    qDebug() << "Entered updateParamTable():" << model;
    QString name = QString("(") + model + QString(" parameters)");
    emit setParamDataFrameSignal(name);
    emit newTableSelection(name);
}

void TableEditor::resizeColumns()
{
    view->resizeColumnsToContents();

//    for (int i=0; i<view->columns();i++)
//        setColumnWidth(i, .view->width() / columns());
}

void TableEditor::on_copy_clicked()
{
    QAbstractItemModel *abmodel = view->model();
    QItemSelectionModel *model = view->selectionModel();
    QModelIndexList list = model->selectedIndexes();

    if(list.size() < 2)
    {
//         select all
                list.clear();
                for(int i=0;i<abmodel->rowCount();i++)
                    for(int j=0;j<abmodel->columnCount();j++)
                        list.append(abmodel->index(i,j));
    }
    qSort(list);
    qDebug() << list;

    QString copy_table;
    QModelIndex last = list.last();
    QModelIndex previous = list.first();

    list.removeFirst();

    for(int i = 0; i < list.size(); i++)
    {
        QVariant data = abmodel->data(previous);
        QString text = data.toString();

        QModelIndex index = list.at(i);
        copy_table.append(text);

        if(index.row() != previous.row())

        {
            copy_table.append('\n');
        }
        else
        {
            copy_table.append('\t');
        }
        previous = index;
    }

    copy_table.append(abmodel->data(list.last()).toString());
    copy_table.append('\n');

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(copy_table);

}
