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

#include "tableviewer2.h"
#include "dataframemodel.h"
#include "objectcatalogue.h"
#include "objectcataloguefilter.h"
#include "lazynutjobparam.h"
#include "lazynutjob.h"
#include "sessionmanager.h"
#include "finddialog.h"
#include "settingsform.h"
#include "settingsformdialog.h"

TableViewer::TableViewer(const QString &tableName, QWidget *parent)
    : QMainWindow(parent)
{
    // this type of form is used for stimSetForm and results tables

    createActions();
    createToolBars();

    tablePanel = new QTabWidget;
    numTables = 0;
    currentTableIdx = 0;
//    widget = new QWidget(this);
//    setCentralWidget(widget);
    setCentralWidget(tablePanel);
//    QVBoxLayout* layout = new QVBoxLayout(widget);
//    layout->addWidget(tableBox);
//    layout->addWidget(view);
//    widget->setLayout(layout);
//    addTable("bla");
}

void TableViewer::addTrialTable(QString name)
{
    QString tableName = addTable(name); // this adds the tab, if it's unique
    QString defaultTableName = QString("((") + name + QString(" default_observer) default_dataframe)");
//    qDebug() << "defaultTableName =" << defaultTableName;
//    qDebug() << "tableName =" << tableName;
    QString cmd = defaultTableName + QString(" copy ") + tableName;
    SessionManager::instance()->runCmd(cmd);
}

void TableViewer::addTableWithThisName(QString name)
{
    addTable(name, false);
}

QString TableViewer::addTable(QString name, bool chooseNewName)
{
//    qDebug() << "addTable(" << name << ")";
    if (name.isEmpty())
        name = tr("Table_")+QString::number(numTables);
    else if (chooseNewName)
        name.append(".table");
//    qDebug() << "current list: " << tableMap.values();
    if (tableMap.values().contains(name))
        return(name);
//    qDebug() << "going to add a new one";
    tables.push_back(new QTableView(this));
    myHeaders.push_back(new DataFrameHeader(tables.back()));
    numTables++;
    currentTableIdx = numTables-1;
    int idx = tablePanel->addTab(tables[numTables-1], name);
//    qDebug() << "adding table to panel. New numTables = " << numTables;
    tablePanel->setCurrentIndex(idx);
    tableMap[idx] = name;
//    qDebug() << name << "has idx " << idx;

    tables[tablePanel->currentIndex()]->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // stimulus set allows a column to be dragged
    tables[tablePanel->currentIndex()]->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tables[tablePanel->currentIndex()]->setDragEnabled(true);
    tables[tablePanel->currentIndex()]->setDropIndicatorShown(true);
    tables[tablePanel->currentIndex()]->setDragDropMode(QAbstractItemView::DragOnly);

    tables[tablePanel->currentIndex()]->setHorizontalHeader(myHeaders[tablePanel->currentIndex()]);
    connect(myHeaders[tablePanel->currentIndex()], SIGNAL(columnDropped(QString)), this, SIGNAL(columnDropped(QString)));
    connect(myHeaders[tablePanel->currentIndex()], SIGNAL(restoreComboBoxText()), this, SIGNAL(restoreComboBoxText()));
    connect(this,SIGNAL(newTableName(QString)),myHeaders[tablePanel->currentIndex()],SLOT(setTableName(QString)));

    return name;
}

void TableViewer::switchTab(QString tableName)
{
    int idx = tableMap.key(tableName);
    tablePanel->setCurrentIndex(idx);
}

void TableViewer::setTableText(QString text)
{
    tableBox->addItem(text);
    tableBox->setCurrentIndex(tableBox->findData(text,Qt::DisplayRole));
    emit newTableName(text);
}

void TableViewer::createActions()
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

    mergeDFAct = new QAction(QIcon(":/images/Merge_Icon.png"), tr("&Merge two dataframes"), this);
    mergeDFAct->setStatusTip(tr("Merge two dataframes"));
    connect(mergeDFAct, SIGNAL(triggered()), this, SLOT(mergeFD()));
    mergeDFAct->setEnabled(true);

    findAct = new QAction(QIcon(":/images/magnifying-glass-2x.png"), tr("&Find"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setToolTip(tr("Find text in this table"));
    connect(findAct, SIGNAL(triggered()), this, SLOT(showFindDialog()));
    findAct->setEnabled(true);

    plotAct = new QAction(QIcon(":/images/barchart2.png"), tr("&Plot"), this);
    plotAct->setToolTip(tr("Create a plot based on these data"));
//    connect(plotAct, SIGNAL(triggered()), this, SLOT(preparePlot()));
    plotAct->setEnabled(true);

    QMenu *menu = new QMenu();
    QAction *barchartAct = new QAction("barchart", this);
    QAction *activityAct = new QAction("activity", this);
    menu->addAction(barchartAct);
    menu->addAction(activityAct);
    connect(barchartAct, SIGNAL(triggered()), this, SLOT(preparePlot()));

    plotAct->setMenu(menu);
}

void TableViewer::refresh()
{
    updateTableView(tableMap[tablePanel->currentIndex()]);

}

void TableViewer::preparePlot()
{
    if (tableMap[tablePanel->currentIndex()].isEmpty())
        return;
    QMap<QString,QString> settings;
    settings["df"]=tableMap[tablePanel->currentIndex()];
    QString plotName = tableMap[tablePanel->currentIndex()].append(".plot");
    QString plotType = "plot_mean_bars.R"; // testing!!!
    emit createNewPlotOfType(plotName, plotType, settings);
    emit showPlotSettings();
}

void TableViewer::save()
{
    QSettings settings("QtEasyNet", "nmConsole");
    QString workingDir = settings.value("easyNetHome").toString();
    QString currFilename = workingDir + "/Output_files/.";
    qDebug() << "df currFilename" << currFilename;
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Choose a file name"), currFilename,
                        tr("CSV (*.csv)"));
    if (fileName.isEmpty())
        return;

    QString cmd = tableMap[tablePanel->currentIndex()] + " save_csv " + fileName;
    SessionManager::instance()->runCmd(cmd);

}

void TableViewer::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    if (openAct)
        fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(refreshAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(copyDFAct);
    editToolBar->addAction(mergeDFAct);
    editToolBar->addAction(findAct);
//    editToolBar->addAction(plotAct);

    fileToolBar->setMovable(false);
    editToolBar->setMovable(false);

//    QMenu *menu = new QMenu();
////    QAction *testAction = new QAction("barchart", this);
//    menu->addAction(plotAct);

//    QToolButton* plotButton = new QToolButton(this);
//    plotButton->setIcon(QIcon(":/images/barchart2.png"));
//    plotButton->setToolTip(tr("Create a plot based on these data"));
//    plotButton->setMenu(menu);
//    plotButton->setPopupMode(QToolButton::InstantPopup);
//    editToolBar->addWidget(plotButton);

    editToolBar->addAction(plotAct);
}

void TableViewer::setView(QString name)
{
    emit newTableSelection(name);
}

void TableViewer::addDataFrameToWidget(QDomDocument* domDoc, QString cmd)
{
    QString tableName = cmd.remove(QRegExp(" get.*$")).remove(QRegExp("^.*xml")).simplified();
    int idx = tableMap.key(tableName);

    // leakage: need to delete model and model viewer
    dfModel = new DataFrameModel(domDoc, this); // you only need this line to load in the entire XML table

    tables[idx]->setModel(dfModel);
    tables[idx]->resizeColumnsToContents();
    tables[idx]->show();
    // at this point we have a view widget showing the table
    tables[idx]->verticalHeader()->hide(); // hideColumn(0); // 1st column contains rownames, which user doesn't need
    QItemSelectionModel* selModel = tables[idx]->selectionModel();
    connect(selModel, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this,SLOT(rowChangedSlot( const QModelIndex& , const QModelIndex& )));
}

void TableViewer::replaceHeaders(QTableView* view)
{
    QStringList headers, newHeaders;
    for(int i = 0; i < view->model()->columnCount(); i++)
        headers.append(view->model()->headerData(i, Qt::Horizontal).toString());

//    qDebug() << "Here are the original headers" << headers;

    QString trial = "ldt"; // for example
    int col=0;
    foreach(QString header,headers)
    {
        header.replace("(","");
        header.replace(")","");
        header.replace("event_pattern","");
        header.replace(trial,"");
        newHeaders.append(header.simplified());
        QVariant qv(header.simplified());

        /* DOESN'T WORK */
        view->model()->setHeaderData(col, Qt::Horizontal,qv);
        col++;

    }
//    qDebug() << "Here are the replaced headers" << newHeaders;

}



void TableViewer::rowChangedSlot( const QModelIndex& selected, const QModelIndex& deselected )
{
    QString key = (tables[tablePanel->currentIndex()]->model()->data(selected.sibling(selected.row(), 0) ,0)).toString();
//    qDebug() << key;
    emit currentKeyChanged(key);

}

void TableViewer::updateParamTable(QString model)
{
    if (model.isEmpty())
        return;
//    qDebug() << "Entered updateParamTable():" << model;
    QString name = QString("(") + model + QString(" parameters)");
    emit setParamDataFrameSignal(name);
    updateTableView(name);
}

void TableViewer::resizeColumns()
{
    tables[tablePanel->currentIndex()]->resizeColumnsToContents();

//    for (int i=0; i<view->columns();i++)
//        setColumnWidth(i, .view->width() / columns());
}

void TableViewer::on_copy_clicked()
{
    // a different approach -- get R to copy the df to the clipboard

    QString cmd = "R << write.table(eN[\"THEDATAFRAME\"], \"clipboard\", sep=\"\t\", row.names=FALSE)";
    cmd.replace("THEDATAFRAME",tableMap[tablePanel->currentIndex()]);
    SessionManager::instance()->runCmd(cmd);
    return;

    QAbstractItemModel *abmodel = tables[tablePanel->currentIndex()]->model();
    QItemSelectionModel *model = tables[tablePanel->currentIndex()]->selectionModel();
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
//    qDebug() << list;

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

void TableViewer::on_copy_DF_clicked()
{
    QString cmd = tableMap[tablePanel->currentIndex()] + " copy " + "new_df";
    SessionManager::instance()->runCmd(cmd);

}

void TableViewer::mergeFD()
{
    // load XML
    QDomDocument* domDoc = new QDomDocument;
    QSettings settings("QtEasyNet", "nmConsole");
    QString easyNetHome = settings.value("easyNetHome","../..").toString();
    QFile file(QString("%1/XML_files/dataframe_merge.xml").arg(easyNetHome));
    if (!file.open(QIODevice::ReadOnly))
        return;
    if (!domDoc->setContent(&file)) {
        file.close();
        return;
    }
    file.close();
    // setup form
    SettingsForm *form = new SettingsForm(domDoc, this);
    form->setUseRFormat(false);
    QMap<QString, QString> preFilledSettings;
    preFilledSettings["x"] = tablePanel->tabText(tablePanel->currentIndex());
    preFilledSettings["y"] = SessionManager::instance()->currentSet();
    form->setDefaultSettings(preFilledSettings);
    // setup dialog
    QString info("Select two dataframes you want to merge into one. Their key columns should match.");
    SettingsFormDialog dialog(domDoc, form, info, this);


    connect(&dialog, &SettingsFormDialog::cmdListReady, [=](QStringList cmdList)
    {
        SessionManager::instance()->runCmd(cmdList);
    });
    connect(&dialog, &SettingsFormDialog::dfNameReady, [=](QString dfName)
    {
       addTableWithThisName(dfName);
       updateTableView(dfName);
    });

    dialog.exec();
}


void TableViewer::dragEnterEvent(QDragEnterEvent *event)
{
//    qDebug() << "drageenterevent";
    if (event->mimeData()->hasFormat("application/x-dnditemdata"))
    {
        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->acceptProposedAction();
        }
    }
    else
    {
        event->ignore();
    }
}

void TableViewer::dragMoveEvent(QDragMoveEvent *event)
{
//if (event->mimeData()->hasFormat("application/x-dnditemdata"))
{
    if (event->source() == this) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else {
        event->acceptProposedAction();
    }
}
//else {
//    event->ignore();
//}
}

//! [1]
void TableViewer::mousePressEvent(QMouseEvent *event)
{
QTableView *child = static_cast<QTableView*>(childAt(event->pos()));
if (!child)
    return;

qDebug("child name %s", child->objectName().toLatin1().data());

//if (!child->text().isEmpty())
//{
//    draggedText = child->text();
//    qDebug() << "draggedText" << draggedText;
//}

QByteArray itemData;
QDataStream dataStream(&itemData, QIODevice::WriteOnly);
//dataStream << pixmap << QPoint(event->pos() - child->pos());
//! [1]

//! [2]
QMimeData *mimeData = new QMimeData;
mimeData->setData("application/x-dnditemdata", itemData);
//! [2]

//! [3]
QDrag *drag = new QDrag(this);
drag->setMimeData(mimeData);
drag->setHotSpot(event->pos() - child->pos());
//! [3]


if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction)
    child->close();
else
    child->show();
}

bool TableViewer::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
  QStringList formats = data->formats();
  QByteArray encodedData = data->data(formats[0]);
  QDataStream stream(&encodedData, QIODevice::ReadOnly);

//  int row, column;
  stream >> row >> column;

  qDebug() << "row: " << row << " column:" << column;

  return false;
}

void TableViewer::updateTableView(QString text)
{
//    qDebug() << this << "Entered updateTableView with " << text;
//    qDebug() << "currentIndex = " << tables[tablePanel->currentIndex()]->currentIndex();
    if (text.isEmpty())
        return;
    if (text=="Untitled")
        return;

//    if (text==tableMap[tablePanel->currentIndex()])
//        return;

    currentTable = text;
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode &= ECHO_INTERPRETER;
    param->cmdList = QStringList({QString("xml " + text + " get")});
    param->answerFormatterType = AnswerFormatterType::XML;
    param->setAnswerReceiver(this, SLOT(addDataFrameToWidget(QDomDocument*, QString)));
    SessionManager::instance()->setupJob(param, sender());
}


void TableViewer::showFindDialog()
{
    findDialog = new FindDialog;
    findDialog->hideExtendedOptions();
    connect(findDialog, SIGNAL(findForward(QString, QFlags<QTextDocument::FindFlag>)),
            this, SLOT(findForward(QString, QFlags<QTextDocument::FindFlag>)));

    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
}

void TableViewer::findForward(const QString &str, QFlags<QTextDocument::FindFlag> flags)
{
    QFlags<Qt::MatchFlag> flag;
//    if (flags |= QTextDocument::FindCaseSensitively)
//        flag = Qt::MatchCaseSensitive;
//    else
        flag = Qt::MatchExactly;
    QVariant qv(str);

    // first try searching in the current column
    int row = tables[tablePanel->currentIndex()]->currentIndex().row();
    int col = tables[tablePanel->currentIndex()]->currentIndex().column();
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
        tables[tablePanel->currentIndex()]->setCurrentIndex(hits.first());
//        findDialog->hide();
    }
    else
        QMessageBox::warning(this, "Find",QString("The text was not found"));
//        findDialog->hide();

}


