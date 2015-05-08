#include <QtWidgets>
#include <QtGlobal>
#include <QtDebug>
#include <QComboBox>
#include <QStringList>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QSplitter>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>


#include "objexplorer.h"
#include "lazynutobject.h"
#include "sessionmanager.h"
#include "lazynutjobparam.h"
#include "xmlelement.h"


ObjExplorer::ObjExplorer(LazyNutObjectCatalogue *objectCatalogue, QWidget *parent)
    : objectCatalogue(objectCatalogue), QMainWindow(parent)
{
    createTaxonomy();
    QSplitter *splitter = new QSplitter;
    splitter->setOrientation(Qt::Horizontal);

//    lazyNutObjView = new QTableView;
    lazyNutObjectView = new QTreeView;
    lazyNutObjectView->header()->hide();
    lazyNutObjectView->expandAll();
    lazyNutObjectView->header()->setStretchLastSection(true);
    lazyNutObjectView->setSelectionMode(QAbstractItemView::SingleSelection);

//    objTaxonomyView = new QTreeView;
//    objTaxonomyView->setModel(objTaxonomyModel);
//    objTaxonomyView->setSelectionMode(QAbstractItemView::SingleSelection);


//    connect(objTaxonomyView,SIGNAL(clicked(QModelIndex)),
//            objTaxonomyModel,SLOT(getGenealogy(QModelIndex)));

    lazyNutObjTableModel = new LazyNutObjTableModel(objectCatalogue,this);
//    lazyNutObjectListModel = new LazyNutObjectListModel(objectCatalogue,this);


//    connect(this,SIGNAL(beginObjHashModified()),lazyNutObjTableModel,SLOT(sendBeginResetModel()));
//    connect(this,SIGNAL(endObjHashModified()),lazyNutObjTableModel,SLOT(sendEndResetModel()));


    lazyNutObjTableView = new QTableView;
    lazyNutObjTableView->setModel(lazyNutObjTableModel);

//    lazyNutObjectListView = new QListView;
//    lazyNutObjectListView->setModel(lazyNutObjectListModel);
//    connect(lazyNutObjectListView,SIGNAL(clicked(QModelIndex)),
//            lazyNutObjectListModel,SLOT(getObjFromListIndex(QModelIndex)));
//    connect(lazyNutObjectListModel,SIGNAL(objectRequested(QString)),
//            this,SLOT(setObjFromObjName(QString)));

    lazyNutObjTableProxyModel = new LazyNutObjTableProxyModel(this);
    lazyNutObjTableProxyModel->setDynamicSortFilter(true);
    //lazyNutObjTableProxyModel->setFilterKeyColumns(QList<int>({1}));
    //lazyNutObjTableProxyModel->addFilterFixedString(1,"layer");
    lazyNutObjTableProxyModel->setSourceModel(lazyNutObjTableModel);


    lazyNutObjTableProxyView = new QTableView(this);
    lazyNutObjTableProxyView->setModel(lazyNutObjTableProxyModel);
    lazyNutObjTableProxyView->setSortingEnabled(true);
    lazyNutObjTableProxyView->sortByColumn(0, Qt::AscendingOrder);
    lazyNutObjTableProxyView->setColumnHidden(1,true);
    lazyNutObjTableProxyView->setColumnHidden(2,true);
    lazyNutObjTableProxyView->setSelectionMode(QAbstractItemView::SingleSelection);
    lazyNutObjTableProxyView->horizontalHeader()->setStretchLastSection(true);

    connect(lazyNutObjTableProxyView,SIGNAL(clicked(QModelIndex)),
            lazyNutObjTableProxyModel,SLOT(mapIndexToSourceModel(QModelIndex)));
    connect(lazyNutObjTableProxyModel,SIGNAL(getObj(QModelIndex)),
            this, SLOT(setObjFromListIndex(QModelIndex)));

    connectTaxonomyModel();


    splitter->addWidget(taxWidget);
    splitter->addWidget(lazyNutObjTableProxyView);
//    splitter->addWidget(lazyNutObjectListView);
    splitter->addWidget(lazyNutObjectView);

//    lazyNutObjModel = new LazyNutObjModel(new LazyNutObj,objHash,this);


//    connect(lazyNutObjTableProxyModel,SIGNAL(getObj(const QModelIndex&)),
//            lazyNutObjModel, SLOT(getObjFromCatalogueIndex(const QModelIndex&)));
//    connect(lazyNutObjModel, SIGNAL(showObj(LazyNutObj*, LazyNutObjCatalogue*)),
//            this, SLOT(setObj(LazyNutObj*, LazyNutObjCatalogue*)));

 //   setObj((*objHash)["layerA"],objHash);

    setWindowTitle(tr("Object Explorer"));
    setCentralWidget(splitter);

}

//void ObjExplorer::setObj(LazyNutObj* obj, LazyNutObjCatalogue *objHash)
//{
//    delete lazyNutObjModel;
//    lazyNutObjModel = new LazyNutObjModel(obj,objHash,this);
//    comboBoxDelegate = new ComboBoxDelegate(objHash,this);
//    lazyNutObjectView->setModel(lazyNutObjModel);

//    foreach(int row, obj->rowsWithStringList())
//    {
//        lazyNutObjectView->setItemDelegateForRow(row,comboBoxDelegate);
//        lazyNutObjectView->openPersistentEditor(lazyNutObjModel->index(row));
//    }

//    lazyNutObjectView->resizeRowsToContents();

//    connect(comboBoxDelegate,SIGNAL(activatedDelegateIndex(const QModelIndex&,int)),
//            lazyNutObjModel, SLOT(getObjFromDescriptionIndex(const QModelIndex,int)));
//    connect(lazyNutObjectView, SIGNAL(clicked(const QModelIndex&)),
//            lazyNutObjModel, SLOT(getObjFromDescriptionIndex(const QModelIndex&)));
//    connect(lazyNutObjTableProxyModel,SIGNAL(getObj(const QModelIndex&)),
//            lazyNutObjModel, SLOT(getObjFromCatalogueIndex(const QModelIndex&)));
//    connect(lazyNutObjModel, SIGNAL(showObj(LazyNutObj*, LazyNutObjCatalogue*)),
//            this, SLOT(setObj(LazyNutObj*, LazyNutObjCatalogue*)));

//    emit (objectSelected(objHash->key(obj)));

//}

void ObjExplorer::setObjFromListIndex(QModelIndex index)
{
//    setObjFromObjName(lazyNutObjectListModel->data(index,Qt::DisplayRole).toString());
    setObjFromObjName(lazyNutObjTableModel->data(index,Qt::DisplayRole).toString());

}

void ObjExplorer::setObjFromObjName(QString name)
{
    lazyNutObject = objectCatalogue->value(name);
    lazyNutObjectView->setModel(lazyNutObject);
    lazyNutObjectView->expandAll();
    lazyNutObjectView->resizeColumnToContents(0);
    lazyNutObjectView->resizeColumnToContents(1);
    emit objectSelected(name);
}

void ObjExplorer::updateLazyNutObjCatalogue(QDomDocument *domDoc)
{
    LazyNutObject *newObj = new LazyNutObject(domDoc, this);
    QString name = newObj->name();
//    lazyNutObjTableModel->sendBeginResetModel();
    delete objectCatalogue->value(name);
    objectCatalogue->insert(name,newObj);
    connect(lazyNutObjectView,SIGNAL(clicked(QModelIndex)),
            newObj,SLOT(getObjFromDescriptionIndex(QModelIndex)));
    connect(newObj,SIGNAL(objectRequested(QString)),
            this,SLOT(setObjFromObjName(QString)));
//    lazyNutObjTableModel->sendEndResetModel();
    //emit objCatalogueChanged();
}

void ObjExplorer::createTaxonomy()
{
    objTaxonomyModel = new TreeModel(QStringList({"type"}));

    taxListView = new QListView;
    taxColumnView = new QColumnView;
    taxTreeView = new QTreeView;

    taxViewMap["list"] = taxListView;
    taxViewMap["column"] = taxColumnView;
    taxViewMap["tree"] = taxTreeView;

    taxWidget = new QStackedWidget;
    foreach(QString view, taxViewMap.keys())
    {
        taxWidget->addWidget(taxViewMap[view]);
        qobject_cast<QAbstractItemView*>(taxViewMap[view])->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    createTaxonomyMenu();
}

void ObjExplorer::createTaxonomyMenu()
{
    taxonomyMenu = menuBar()->addMenu("Taxonomy");
    taxonomyMenu->addAction("Reload", this, SLOT(reloadTaxonomy()));
    QMenu *taxViewMenu = taxonomyMenu->addMenu("View mode");
    foreach(QString view, taxViewMap.keys())
       taxViewMenu->addAction(view, this, SLOT(setTaxonomyView()));

}

void ObjExplorer::setTaxonomyView()
{
    taxWidget->setCurrentWidget(taxViewMap.value(qobject_cast<QAction*>(sender())->text()));
}

void ObjExplorer::reloadTaxonomy()
{
    if (SessionManager::instance()->isOn())
        getTaxonomyTypes();
}

void ObjExplorer::resetLazyNutObjTableModel()
{
    lazyNutObjTableModel->sendBeginResetModel();
    lazyNutObjTableModel->sendEndResetModel();
}

void ObjExplorer::getTaxonomyTypes()
{
    delete objTaxonomyModel;
    objTaxonomyModel = new TreeModel(QStringList({"type", "subtype"}));
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({"xml list type"});
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(this, SLOT(setTaxonomyTypes(QStringList)));
    param->setNextJobReceiver(this, SLOT(getTaxonomySubtypes()));
    SessionManager::instance()->setupJob(param, sender());
}

void ObjExplorer::setTaxonomyTypes(QStringList types)
{
    foreach (QString type, types)
        objTaxonomyModel->appendValue(type);
}

void ObjExplorer::getTaxonomySubtypes()
{
    QStringList cmdList;
    for (int row = 0; row < objTaxonomyModel->rowCount(); ++row)
        cmdList.append(objTaxonomyModel->data(objTaxonomyModel->index(row, 0), Qt::EditRole).toString());

    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = cmdList.replaceInStrings(QRegExp("^"), "xml subtypes ");
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(this, SLOT(setTaxonomySubtypes(QStringList,QString)));
    param->setEndOfJobReceiver(this, SLOT(connectTaxonomyModel()));
    SessionManager::instance()->setupJob(param, sender());
}

void ObjExplorer::setTaxonomySubtypes(QStringList subtypes, QString cmd)
{
    QRegExp typeNameRex("xml subtypes (\\w+)");
    if (typeNameRex.indexIn(cmd) < 0)
    {
        qDebug() << "no type in ObjExplorer::setTaxonomySubtypes(). cmd = " << cmd;
        return;
    }
    QString typeName = typeNameRex.cap(1);
    QModelIndex typeIndex = objTaxonomyModel->match(objTaxonomyModel->index(0,0), Qt::DisplayRole,
                            typeName, 1, Qt::MatchExactly).first();
    foreach(QString subtype, subtypes)
        objTaxonomyModel->appendValue(subtype, typeIndex);

}

void ObjExplorer::connectTaxonomyModel()
{
    foreach(QString view, taxViewMap.keys())
    {
        qobject_cast<QAbstractItemView*>(taxViewMap[view])->setModel(objTaxonomyModel);
        connect(qobject_cast<QAbstractItemView*>(taxViewMap[view]), SIGNAL(clicked(const QModelIndex&)),
                objTaxonomyModel, SLOT(getGenealogy(const QModelIndex&)));
        connect(objTaxonomyModel,SIGNAL(sendGenealogy(QList<QVariant>)),
                lazyNutObjTableProxyModel,SLOT(setFilterFromGenealogy(QList<QVariant>)));

    }
    taxTreeView->expandAll();
}
