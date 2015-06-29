/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QDomDocument>
#include <QtWidgets>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QtWidgets>
#include <QtSql>


#include "tableeditor.h"
#include "dataframemodel.h"
#include "objectcatalogue.h"
#include "objectcataloguefilter.h"

TableEditor::TableEditor(const QString &tableName, QWidget *parent)
    : QMainWindow(parent)
{
    init(tableName, parent);
//    listTitle->hide();
    objectListView->hide();

}

TableEditor::TableEditor(ObjectCatalogue *_objectCatalogue, const QString &tableName, QWidget *parent)
    : QMainWindow (parent)
{
    init(tableName, parent);
    objectCatalogue = _objectCatalogue;
    setFilter("dataframe");
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

    connect(this,SIGNAL(currentKeyChanged(QString)),
            parent,SLOT(currentStimulusChanged(QString)));

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
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));


    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

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
//    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));
    copyAct->setEnabled(false);
//    connect(textEdit, SIGNAL(copyAvailable(bool)),
//            copyAct, SLOT(setEnabled(bool)));

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


void TableEditor::submit()
{
    model->database().transaction();
    if (model->submitAll()) {
        model->database().commit();
    } else {
        model->database().rollback();
        QMessageBox::warning(this, tr("Cached Table"),
                             tr("The database reported an error: %1")
                             .arg(model->lastError().text()));
    }
}

void TableEditor::setView(QString name)
{
//    tableTitle->setText(name);
    emit newTableSelection(name);
}

void TableEditor::addDataFrameToWidget(QDomDocument* domDoc)
{
    dfModel = new DataFrameModel(domDoc, this); // you only need this line to load in the entire XML table
    view->setModel(dfModel);
    view->resizeColumnsToContents();
    view->show();
    // at this point we have a view widget showing the table

    QItemSelectionModel* selModel = view->selectionModel();
    connect(selModel, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this,SLOT(rowChangedSlot( const QModelIndex& , const QModelIndex& )));
}

void TableEditor::rowChangedSlot( const QModelIndex& selected, const QModelIndex& deselected )
{
    QString key = (view->model()->data(selected.sibling(selected.row(), 0) ,0)).toString();
    qDebug() << key;
    emit currentKeyChanged(key);

}
