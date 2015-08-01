#include "parameditor.h"

#include <QDomDocument>
#include <QtWidgets>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QStandardItemModel>


ParamEditor::ParamEditor(QWidget *parent)
    : QMainWindow(parent)
{
    view = new QTableView(this);
    view->setEditTriggers(QAbstractItemView::AllEditTriggers);

}


