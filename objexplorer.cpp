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
#include "lazynutobj.h"
//#include "comboboxdelegate.h"
#include "lazynutobject.h"


ObjExplorer::ObjExplorer(LazyNutObjectCatalogue *objectCatalogue, QWidget *parent)
    : objectCatalogue(objectCatalogue), QSplitter(parent)
{

    setOrientation(Qt::Vertical);

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

//    lazyNutObjTableModel = new LazyNutObjTableModel(objHash,this);
    lazyNutObjectListModel = new LazyNutObjectListModel(objectCatalogue,this);


    connect(this,SIGNAL(beginObjHashModified()),lazyNutObjectListModel,SLOT(sendBeginResetModel()));
    connect(this,SIGNAL(endObjHashModified()),lazyNutObjectListModel,SLOT(sendEndResetModel()));


//    lazyNutObjTableView = new QTableView;
//    lazyNutObjTableView->setModel(lazyNutObjTableModel);

    lazyNutObjectListView = new QListView;
    lazyNutObjectListView->setModel(lazyNutObjectListModel);
    connect(lazyNutObjectListView,SIGNAL(clicked(QModelIndex)),
            lazyNutObjectListModel,SLOT(getObjFromListIndex(QModelIndex)));
    connect(lazyNutObjectListModel,SIGNAL(objectRequested(QString)),
            this,SLOT(setObjFromObjName(QString)));

//    lazyNutObjTableProxyModel = new LazyNutObjTableProxyModel(this);
//    lazyNutObjTableProxyModel->setDynamicSortFilter(true);
//    //lazyNutObjTableProxyModel->setFilterKeyColumns(QList<int>({1}));
//    //lazyNutObjTableProxyModel->addFilterFixedString(1,"layer");
//    lazyNutObjTableProxyModel->setSourceModel(lazyNutObjTableModel);

//    connect(objTaxonomyModel,SIGNAL(sendGenealogy(QList<QVariant>)),
//            lazyNutObjTableProxyModel,SLOT(setFilterFromGenealogy(QList<QVariant>)));

//    lazyNutObjTableProxyView = new QTableView(this);
//    lazyNutObjTableProxyView->setModel(lazyNutObjTableProxyModel);
//    lazyNutObjTableProxyView->setSortingEnabled(true);
//    lazyNutObjTableProxyView->sortByColumn(0, Qt::AscendingOrder);
//    lazyNutObjTableProxyView->setColumnHidden(1,true);
//    lazyNutObjTableProxyView->setColumnHidden(2,true);
//    lazyNutObjTableProxyView->setSelectionMode(QAbstractItemView::SingleSelection);
//    lazyNutObjTableProxyView->horizontalHeader()->setStretchLastSection(true);

//    connect(lazyNutObjTableProxyView,SIGNAL(clicked(QModelIndex)),
//            lazyNutObjTableProxyModel,SLOT(mapIndexToSourceModel(QModelIndex)));


//    addWidget(objTaxonomyView);
//    addWidget(lazyNutObjTableProxyView);
    addWidget(lazyNutObjectListView);
    addWidget(lazyNutObjectView);

//    lazyNutObjModel = new LazyNutObjModel(new LazyNutObj,objHash,this);


//    connect(lazyNutObjTableProxyModel,SIGNAL(getObj(const QModelIndex&)),
//            lazyNutObjModel, SLOT(getObjFromCatalogueIndex(const QModelIndex&)));
//    connect(lazyNutObjModel, SIGNAL(showObj(LazyNutObj*, LazyNutObjCatalogue*)),
//            this, SLOT(setObj(LazyNutObj*, LazyNutObjCatalogue*)));\

 //   setObj((*objHash)["layerA"],objHash);

    setWindowTitle(tr("Object Explorer"));
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
//            this, SLOT(setObj(LazyNutObj*, LazyNutObjCatalogue*)));\

//    emit (objectSelected(objHash->key(obj)));

//}

void ObjExplorer::setObjFromListIndex(QModelIndex index)
{
    setObjFromObjName(lazyNutObjectListModel->data(index,Qt::DisplayRole).toString());
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
    lazyNutObjectListModel->sendBeginResetModel();
    delete objectCatalogue->value(name);
    objectCatalogue->insert(name,newObj);
    connect(lazyNutObjectView,SIGNAL(clicked(QModelIndex)),
            newObj,SLOT(getObjFromDescriptionIndex(QModelIndex)));
    connect(newObj,SIGNAL(objectRequested(QString)),
            this,SLOT(setObjFromObjName(QString)));
    lazyNutObjectListModel->sendEndResetModel();
    //emit objCatalogueChanged();
}
