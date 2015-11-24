#include "tabstablewidget.h"
#include "dataframemodel.h"
#include "trialdataframemodel.h"
#include "objectcachefilter.h"
#include "sessionmanager.h"
#include "lazynutjob.h"

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

void TabsTableWidget::addTable_impl(QString name)
{
    dataframeFilter->setName(name);
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
        QRegExp prettyNameRx("\\(\\((\\w+) default_observer\\) default_dataframe\\)");
        QString prettyName = prettyNameRx.indexIn(name) != -1 ? prettyNameRx.cap(1) : name;
        tabWidget->addTab(dFmodel->view(), prettyName);
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
    if (isNewModel)
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
//    emit tableDeleted(name);
    if (!name.contains(QRegExp("[()]")))
    {
        LazyNutJob *job = new LazyNutJob;
        job->logMode |= ECHO_INTERPRETER;
        job->cmdList << QString("destroy %1").arg(name);
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job
                << SessionManager::instance()->recentlyDestroyedJob();
        SessionManager::instance()->submitJobs(jobs);
    }
}

void TabsTableWidget::buildWidget()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    tabWidget = new QTabWidget;
    tabWidget->setTabsClosable(true);
    connect(tabWidget->tabBar(), &QTabBar::tabCloseRequested, [=](int index)
    {
       deleteTable(tableAt(index));
    });
    connect(tabWidget, &QTabWidget::currentChanged, [=](int index)
    {
        emit currentTableChanged(tableAt(index));
    });
    layout->addWidget(tabWidget);
    setLayout(layout);

}

