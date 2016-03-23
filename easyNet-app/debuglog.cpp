#include <QtWidgets>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QDateTime>

#include "debuglog.h"
#include "finddialog.h"
#include "sessionmanager.h"


DebugLog::DebugLog(QWidget *parent)
        : QMainWindow (parent)
{
    createActions();
    createToolBars();

    model = new QStandardItemModel();
    model->setHorizontalHeaderLabels({"Index","Command","Time"});
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);

    view = new QTableView(this);
    view->setModel(proxyModel);
    view->resizeColumnsToContents();
    view->verticalHeader()->hide(); // hideColumn(0); // 1st column contains rownames, which user doesn't need
    view->show();
    view->setSortingEnabled(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    setCentralWidget(view);

    findDialog = new FindDialog;
    findDialog->hideExtendedOptions();
    connect(findDialog, SIGNAL(findForward(QString, QFlags<QTextDocument::FindFlag>)),
            this, SLOT(findForward(QString, QFlags<QTextDocument::FindFlag>)));
    connect(SessionManager::instance(), SIGNAL(lazyNutCrash()), this, SLOT(autoSave()));

}

DebugLog::~DebugLog()
{

}

void DebugLog::addRowToTable(QString cmd, QString time)
{
    QList<QStandardItem *> rowData;
    int i = model->rowCount(); // + 1;
    QString number = QString("%1").arg(i, 4, 10, QChar('0'));
    rowData << new QStandardItem(number);
    rowData << new QStandardItem(cmd);
    rowData << new QStandardItem(time);
    model->appendRow(rowData);
    view->resizeColumnsToContents();
}

void DebugLog::on_copy_clicked()
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


void DebugLog::createActions()
{
    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(on_copy_clicked()));
    copyAct->setEnabled(true);

    findAct = new QAction(QIcon(":/images/magnifying-glass-2x.png"), tr("&Find"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setToolTip(tr("Find text in this table"));
    connect(findAct, SIGNAL(triggered()), this, SLOT(showFindDialog()));
    findAct->setEnabled(true);
}

QString DebugLog::defaultLogFileName()
{
    QString logDir = SessionManager::instance()->defaultLocation("outputDir");
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy.MM.dd.hh.mm.ss");
    return QString("%1/debug_log.%2.log").arg(logDir).arg(timeStamp);
}


void DebugLog::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(copyAct);
    editToolBar->addAction(findAct);

    fileToolBar->setMovable(false);
    editToolBar->setMovable(false);
}

void DebugLog::showFindDialog()
{

    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
}

void DebugLog::findForward(const QString &str, QFlags<QTextDocument::FindFlag> flags)
{
    qDebug() << "findForward";
    QFlags<Qt::MatchFlag> flag;
//    if (flags |= QTextDocument::FindCaseSensitively)
//        flag = Qt::MatchCaseSensitive;
//    else
        flag = Qt::MatchRegExp;
    QVariant qv(str);

    // first try searching in the current column
    int row = view->currentIndex().row();
    int col = view->currentIndex().column();

    QModelIndexList hits = proxyModel->match(proxyModel->index(row, col),
                            Qt::DisplayRole,qv,1,flag);
    if (hits.size() == 0)
    {
        //now try a more systematic approach
        for (int i=0;i<proxyModel->columnCount();i++)
        {
            hits = proxyModel->match(proxyModel->index(0, i),
                                Qt::DisplayRole,qv);
            if (hits.size() > 0)
                break;
        }
    }

    if (hits.size() > 0)
    {
        qDebug() << "match was " << hits.first();
        view->setCurrentIndex(hits.first());
//        findDialog->hide();
    }
    else
    {
        QMessageBox::warning(this, "Find",QString("The text was not found"));
        findDialog->hide();
    }
}

void DebugLog::save()
{

    QString fileName = QFileDialog::getSaveFileName(this,tr("Save debug log to file"),
                                                    defaultLogFileName(),
                                                    tr("log files (*.log)"));
    if (!fileName.isEmpty())
    {
         saveLogToFile(fileName);
    }
}

void DebugLog::saveLogToFile(QString fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    for (int row = 0; row < model->rowCount(); ++row)
    {
        file.write(QString("%1\t%2\t%3\n")
                   .arg(model->data(model->index(row, 0)).toString())
                   .arg(model->data(model->index(row, 1)).toString())
                   .arg(model->data(model->index(row, 2)).toString()).toLocal8Bit());
    }
    file.close();
}

