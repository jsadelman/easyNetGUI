#include <QDomDocument>
#include <QtWidgets>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QtSql>
#include <QStandardItemModel>
#include <QMimeData>

#include "tableeditor.h"
#include "dataframemodel.h"
#include "objectcache.h"
#include "objectcachefilter.h"
#include "lazynutjob.h"
#include "sessionmanager.h"
#include "finddialog.h"


TableEditor::TableEditor(const QString &tableName, QWidget *parent)
    : dfModel(nullptr), QMainWindow(parent)//,fileToolBar(0),editToolBar(0),tableBox(0)
{
    tableBox=nullptr;
    thisIsParamWindow=false;
    if (tableName=="Parameters")
        thisIsParamWindow=true;
    // this style of constructor is used for stimSetForm
    init(tableName, parent);
//    listTitle->hide();
//    objectListView->hide();
    connect(this,SIGNAL(currentKeyChanged(QString)),
            parent,SLOT(currentStimulusChanged(QString)));
    // can't edit stimulus sets (at present)
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // but can edit parameter sets
    if (thisIsParamWindow)
        view->setEditTriggers(QAbstractItemView::AllEditTriggers);

    else
    {
        // stimulus set allows a column to be dragged
        view->setSelectionMode(QAbstractItemView::ExtendedSelection);
        view->setDragEnabled(true);
        view->setDropIndicatorShown(true);
        view->setDragDropMode(QAbstractItemView::DragOnly);

        DataFrameHeader* myHeader = new DataFrameHeader(view);
        view->setHorizontalHeader(myHeader);
        connect(myHeader, SIGNAL(columnDropped(QString)), this, SIGNAL(columnDropped(QString)));
        connect(myHeader, SIGNAL(restoreComboBoxText()), this, SIGNAL(restoreComboBoxText()));
        connect(this,SIGNAL(newTableName(QString)),myHeader,SLOT(setTableName(QString)));
        connect(this,SIGNAL(newTableName(QString)),myHeader,SLOT(setTableName(QString)));

    }


}

TableEditor::TableEditor(ObjectCache *objectCache, const QString &tableName, QWidget *parent)
    : dfModel(nullptr), objectCache(objectCache), QMainWindow (parent)//,fileToolBar(0),editToolBar(0),tableBox(0)
{
    // this style of constructor is used for dataframes in the Tables tab
    tableBox=nullptr;
    thisIsParamWindow=false;
    if (tableName=="Parameters")
        thisIsParamWindow=true;

    init(tableName, parent);
    setFilter("dataframe");
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

}

void TableEditor::init(const QString &tableName, QWidget *parent)
{
    createActions();
    createToolBars();

    widget = new QWidget(this);
    setCentralWidget(widget);
    QVBoxLayout* layout = new QVBoxLayout(widget);

//    qDebug() << "In TableEditor::init() tableName is " << tableName << "is it param window:" << thisIsParamWindow;
    if (!thisIsParamWindow)
    {
        tableBox = new QComboBox(widget);
        layout->addWidget(tableBox);
//        connect(tableBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateTableView(QString)));
        // don't use currentIndexChanged(), because it'll lead to lots of gets during model load
        connect(tableBox, SIGNAL(activated(QString)), this, SLOT(updateTableView(QString)));
    }

    view = new QTableView(this);
//    objectListView = new QListView(this);
    layout->addWidget(view);
//    layout->addWidget(objectListView);
    widget->setLayout(layout);


}

void TableEditor::setTableText(QString text)
{
    if (tableBox)
    {
        tableBox->addItem(text);
        tableBox->setCurrentIndex(tableBox->findData(text,Qt::DisplayRole));
    }
    emit newTableName(text);

}


void TableEditor::selectTable(QString text)
{
    qDebug() << "Received request to select " << text;
    if (tableBox)
    {
        tableBox->setCurrentIndex(tableBox->findData(text,Qt::DisplayRole));
        qDebug() << "Selected table is" << tableBox->currentText();
    }
}

void TableEditor::setFilter(QString type)
{
    qDebug() << "Entered setFilter with type: " << type << "; tableBox = " << tableBox;
    objectListFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    objectListFilter->setType(type);
    tableBox->setModel(objectListFilter);

}

void TableEditor::createActions()
{
    refreshAct = new QAction(QIcon(":/images/refresh.png"), tr("&Refresh"), this);
    refreshAct->setShortcut(QKeySequence::Refresh);
    refreshAct->setStatusTip(tr("Refresh table"));
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refresh()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SIGNAL(openFileRequest()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    copyAct = new QAction(QIcon(":/images/clipboard.png"), tr("&Copy to clipboard"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(on_copy_clicked()));
    copyAct->setEnabled(true);

    copyDFAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy to new dataframe"), this);
    copyDFAct->setStatusTip(tr("Copy contents to a new dataframe "));
    connect(copyDFAct, SIGNAL(triggered()), this, SLOT(on_copy_DF_clicked()));
    copyDFAct->setEnabled(true);

    findAct = new QAction(QIcon(":/images/magnifying-glass-2x.png"), tr("&Find"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setToolTip(tr("Find text in this table"));
    connect(findAct, SIGNAL(triggered()), this, SLOT(showFindDialog()));
    findAct->setEnabled(true);

}

void TableEditor::refresh()
{
    if (thisIsParamWindow)
        updateTableView(currentTable);
    else
        updateTableView(tableBox->currentText());
}

void TableEditor::save()
{
    QSettings settings("QtEasyNet", "nmConsole");
    QString workingDir = settings.value("easyNetDataHome").toString();
    QString currFilename = workingDir + "/Output_files/.";
    qDebug() << "df currFilename" << currFilename;
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Choose a file name"), currFilename,
                        tr("CSV (*.csv)"));
    if (fileName.isEmpty())
        return;

    QString cmd = currentTable + " save_csv " + fileName;
    SessionManager::instance()->runCmd(cmd);

}

void TableEditor::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    if (openAct)
        fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
//    fileToolBar->addAction(saveAsAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(refreshAct);

//    if (!isReadOnly)
//        editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
//    if (!isReadOnly)
//        editToolBar->addAction(pasteAct);
    editToolBar->addAction(copyDFAct);
    editToolBar->addAction(findAct);

    fileToolBar->setMovable(false);
    editToolBar->setMovable(false);

}



void TableEditor::setView(QString name)
{
    qDebug() << "Entered setView";
    emit newTableSelection(name);
}

void TableEditor::addDataFrameToWidget(QDomDocument* domDoc)
{
    delete dfModel;
    dfModel = new DataFrameModel(domDoc, this); // you only need this line to load in the entire XML table
    dfModel->setName(currentTable);
    connect(dfModel, SIGNAL(newParamValueSig(QString,QString)),
            this,SIGNAL(newParamValueSig(QString,QString)));

    view->setModel(dfModel);
    view->resizeColumnsToContents();
    view->show();
    // at this point we have a view widget showing the table
    view->verticalHeader()->hide(); // hideColumn(0); // 1st column contains rownames, which user doesn't need
    QItemSelectionModel* selModel = view->selectionModel();
    connect(selModel, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this,SLOT(rowChangedSlot( const QModelIndex& , const QModelIndex& )));
}




void TableEditor::rowChangedSlot( const QModelIndex& selected, const QModelIndex& deselected )
{
    QString key = (view->model()->data(selected.sibling(selected.row(), 0) ,0)).toString();
    emit currentKeyChanged(key);

}

void TableEditor::updateParamTable(QString model)
{
    if (model.isEmpty())
        return;
    qDebug() << "Entered updateParamTable():" << model;
    QString name = QString("(") + model + QString(" parameters)");
    emit setParamDataFrameSignal(name);
    updateTableView(name);
}

void TableEditor::resizeColumns()
{
    view->resizeColumnsToContents();

//    for (int i=0; i<view->columns();i++)
//        setColumnWidth(i, .view->width() / columns());
}

void TableEditor::on_copy_clicked()
{
    // a different approach -- get R to copy the df to the clipboard

    QString cmd = QString("R << write.table(eN[\"%1\"], \"clipboard\", sep=\"\t\", row.names=FALSE)").arg(currentTable);
    SessionManager::instance()->runCmd(cmd);
    return;

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

void TableEditor::on_copy_DF_clicked()
{
    QString cmd = QString("%1 copy new_df").arg(currentTable);
    SessionManager::instance()->runCmd(cmd);
}


void TableEditor::updateTableView(QString text)
{
//    qDebug() << this << "Entered updateTableView with " << text;
//    qDebug() << "currentIndex = " << view->currentIndex();
//    qDebug() << "is this param window?" << thisIsParamWindow;
    if (!text.size())
        return;
    if (text=="Untitled")
        return;
//    if (text==currentTable)
//        return;

    currentTable = text;
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("xml %1 get 1-80 1-20").arg(text)}); // just enough to fill the screen
    job->setAnswerReceiver(this, SLOT(addDataFrameToWidget(QDomDocument*)), AnswerFormatterType::XML);
    SessionManager::instance()->submitJobs(job);
}


void TableEditor::showFindDialog()
{
    findDialog = new FindDialog;
    findDialog->hideExtendedOptions();
    connect(findDialog, SIGNAL(findForward(QString, QFlags<QTextDocument::FindFlag>)),
            this, SLOT(findForward(QString, QFlags<QTextDocument::FindFlag>)));

    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
}

void TableEditor::findForward(const QString &str, QFlags<QTextDocument::FindFlag> flags)
{
    QFlags<Qt::MatchFlag> flag;
//    if (flags |= QTextDocument::FindCaseSensitively)
//        flag = Qt::MatchCaseSensitive;
//    else
        flag = Qt::MatchExactly;
    QVariant qv(str);

    // first try searching in the current column
    int row = view->currentIndex().row();
    int col = view->currentIndex().column();
    if (row<0)
        row=0;
    if (col<0)
        col=0;

    QModelIndexList hits = dfModel->match(dfModel->index(row, col),
                            Qt::DisplayRole,qv,1,flag);
    if (hits.size() == 0)
    {
        //now try a more systematic approach
        for (int i=0;i<dfModel->columnCount();i++)
        {
            hits = dfModel->match(dfModel->index(0, i),
                                Qt::DisplayRole,qv);
            if (hits.size() > 0)
                break;
        }
    }

    if (hits.size() > 0)
    {
        view->setCurrentIndex(hits.first());
//        findDialog->hide();
    }
    else
        QMessageBox::warning(this, "Find",QString("The text was not found"));
//        findDialog->hide();

}


