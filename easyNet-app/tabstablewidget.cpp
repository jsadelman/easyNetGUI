#include "tabstablewidget.h"
#include "dataframemodel.h"
#include "trialdataframemodel.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "sessionmanager.h"
#include "lazynutjob.h"
#include "xmlaccessor.h"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QTableView>
#include <QDebug>

TabsTableWidget::TabsTableWidget(QWidget *parent)
    : TableWidgetInterface(parent)
{
    buildWidget();
}

QString TabsTableWidget::currentTable()
{
    return tableAt(tabWidget->currentIndex());
}

QString TabsTableWidget::tableAt(int index)
{
    QTableView *view = qobject_cast<QTableView*>(tabWidget->widget(index));
    if (!view)
    {
//        qDebug() << QString("ERROR: TabsTableWidget::tableAt index %1 does not correspond to any view")
//                    .arg(index);
        return QString();
    }
    DataFrameModel *dFmodel = getDataFrameModel(view->model());
    if (!dFmodel)
    {
        qDebug() << QString("ERROR: TabsTableWidget::tableAt view at index %1 does not have an associated DataFrameModel")
                    .arg(index);
        return QString();
    }
    return dFmodel->name();
}

void TabsTableWidget::setCurrentTable(QString name)
{
    if (!modelMap.contains(name))
        return;

    DataFrameModel *dFmodel = getDataFrameModel(modelMap.value(name));
    if (dFmodel)
        tabWidget->setCurrentWidget(dFmodel->view());

    emit currentTableChanged(name);
}

void TabsTableWidget::setTabState(QString name, int state)
{
    setTabState(tabIndexOfTable(name), state);
}

void TabsTableWidget::setTabState(int index, int state)
{
    QIcon icon;
    switch(state)
    {
    case Tab_DefaultState:
        break;
    case Tab_Updating:
        icon = QIcon(":/images/view_refresh.png");
        break;
    case Tab_Ready:
        icon = QIcon(":/images/icon_check_x24green.png");
        break;
    default:
        break;
    }
    tabWidget->setTabIcon(index, icon);
}

void TabsTableWidget::addTable_impl(QString name)
{
    dataframeFilter->addName(name);
    dataframeDescriptionFilter->addName(name);
}


void TabsTableWidget::updateTable_impl(QAbstractItemModel *model)
{
    DataFrameModel *dFmodel = getDataFrameModel(model);
    if (!dFmodel)
    {
        qDebug() << "ERROR: TabsTableWidget::updateTable_impl cannot extract DataFrameModel from argument";
        return;
    }

    QString name = dFmodel->name();
    if (name.isEmpty())
    {
        qDebug() << "ERROR: TabsTableWidget::updateTable_impl empty DataFrameModel name";
        return;
    }

    if (!modelMap.contains(name))
    {
        qDebug() << "ERROR: TabsTableWidget::updateTable_impl model name unknown" << name;
        return;
    }

    bool isNewModel = (modelMap.value(name) == nullptr);
    if (isNewModel)
    {
        dFmodel->setView(new QTableView(this));
        dFmodel->view()->setModel(model);
        tabWidget->addTab(dFmodel->view(), ""); // the (pretty) name on the tag will be set later
        dFmodel->view()->verticalHeader()->hide();
    }
    else
    {
        DataFrameModel *oldDFmodel = modelMap.value(name);
//        TrialDataFrameModel *oldTDFmodel = qobject_cast<TrialDataFrameModel *>(modelMap.value(name));
//        if (oldTDFmodel)
//            delete oldTDFmodel;

        QTableView *view = oldDFmodel->view();
        dFmodel->setView(view);

        QItemSelectionModel *m = view->selectionModel();
        delete oldDFmodel;
        delete m;

        view->setModel(model);
    }
    dFmodel->view()->resizeColumnsToContents();

    modelMap[name] = dFmodel;
//    if (isNewModel)
        setCurrentTable(name);
}

void TabsTableWidget::deleteTable_impl(QString name)
{
    if (!modelMap.contains(name))
    {
        qDebug() << QString("ERROR: TabsTableWidget::deleteTable_impl attemt to delete the non-existing table %1")
                    .arg(name);
        return;
    }

    DataFrameModel *dFmodel = getDataFrameModel(modelMap.value(name));
    if (!dFmodel)
    {
        qDebug() << QString("ERROR: TabsTableWidget::deleteTable_impl table %1 does not have a DataFrameModel")
                    .arg(name);
        return;
    }

    TrialDataFrameModel *tdFmodel = qobject_cast<TrialDataFrameModel *>(modelMap.value(name));

    QTableView *view = dFmodel->view();
    int index = tabWidget->indexOf(view);
    tabWidget->removeTab(index);
    delete view;
    if (tdFmodel)
        delete tdFmodel;

    delete dFmodel;
    modelMap.remove(name);
    // don't destroy default dataframes
    // their names contain brackets
    if (!name.contains(QRegExp("[()]")))
        SessionManager::instance()->destroyObject(name);

    dataframeDescriptionFilter->removeName(name);
}

int TabsTableWidget::tabIndexOfTable(QString name)
{
    if (!modelMap.contains(name))
        return -1;

    DataFrameModel *dFmodel = getDataFrameModel(modelMap.value(name));
    if (!dFmodel)
         return -1;

    return tabWidget->indexOf(dFmodel->view());
}

void TabsTableWidget::buildWidget()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    tabWidget = new QTabWidget;
    tabWidget->setTabsClosable(true);
    connect(tabWidget, &QTabWidget::tabCloseRequested, [=](int index)
    {
       deleteTable(tableAt(index));
    });
    connect(tabWidget, &QTabWidget::currentChanged, [=](int index)
    {
        emit currentTableChanged(tableAt(index));
        emit hasCurrentTable(index != -1);
    });
    layout->addWidget(tabWidget);
    setLayout(layout);
    connect(dataframeDescriptionUpdater, &ObjectUpdater::objectUpdated, [=](QDomDocument* description, QString name)
    {
        QDomElement rootElement = description->documentElement();
        QDomElement prettyNameElement = XMLAccessor::childElement(rootElement, "pretty name");
        QString prettyName = XMLAccessor::value(prettyNameElement);
        tabWidget->setTabText(tabIndexOfTable(name), prettyName);
    });

}

