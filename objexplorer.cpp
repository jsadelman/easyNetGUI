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
#include "comboboxdelegate.h"

ObjExplorer::ObjExplorer(LazyNutObjCatalogue* objHash, TreeModel* objTaxonomyModel, QWidget *parent)
    : QSplitter(parent), objHash(objHash), objTaxonomyModel(objTaxonomyModel)
{

    // populate obj catalogue
    //objHash = new QHash<QString,LazyNutObj*>();

/*     objHash->insert("layerA",new LazyNutObj());
    (*objHash)["layerA"]->appendProperty(QString{"name"},QVariant{"layerA"});
    (*objHash)["layerA"]->appendProperty("type","layer");
    (*objHash)["layerA"]->appendProperty("subtype","iac_layer");
    (*objHash)["layerA"]->appendProperty("length", 24);
    (*objHash)["layerA"]->appendProperty("incoming connections","connectionBA");

    objHash->insert("layerB",new LazyNutObj());
    (*objHash)["layerB"]->appendProperty("name","layerB");
    (*objHash)["layerB"]->appendProperty("type","layer");
    (*objHash)["layerB"]->appendProperty("subtype","iac_layer");
    (*objHash)["layerB"]->appendProperty("length", 24);
    (*objHash)["layerB"]->appendProperty("incoming connections",QStringList{"connectionAB","biasAB"});

    objHash->insert("connectionAB",new LazyNutObj());
    (*objHash)["connectionAB"]->appendProperty("name","connectionAB");
    (*objHash)["connectionAB"]->appendProperty("type","connection");
    (*objHash)["connectionAB"]->appendProperty("subtype","connection");
    (*objHash)["connectionAB"]->appendProperty("length", 24);
    (*objHash)["connectionAB"]->appendProperty("source","layerA");
    (*objHash)["connectionAB"]->appendProperty("destination","layerB");

    objHash->insert("connectionBA",new LazyNutObj());
    (*objHash)["connectionBA"]->appendProperty("name","connectionBA");
    (*objHash)["connectionBA"]->appendProperty("type","connection");
    (*objHash)["connectionBA"]->appendProperty("subtype","connection");
    (*objHash)["connectionBA"]->appendProperty("length", 24);
    (*objHash)["connectionBA"]->appendProperty("source","layerB");
    (*objHash)["connectionBA"]->appendProperty("destination","layerA");

    objHash->insert("biasAB",new LazyNutObj());
    (*objHash)["biasAB"]->appendProperty("name","biasAB");
    (*objHash)["biasAB"]->appendProperty("type","connection");
    (*objHash)["biasAB"]->appendProperty("subtype","bias");
    (*objHash)["biasAB"]->appendProperty("length", 24);
    (*objHash)["biasAB"]->appendProperty("foo list",QStringList{"foo1","foo2"});
    (*objHash)["biasAB"]->appendProperty("source","layerA");
    (*objHash)["biasAB"]->appendProperty("destination","layerB");


    // populate obj taxonomy
    objTaxonomyModel = new TreeModel(QStringList{"Object taxonomy"},this);
    // root, which is the tip of taxonomy, not the model root (which is not shown in views)
    objTaxonomyModel->appendValue(QString{"object"});
    // type
    QModelIndex parentIndex = objTaxonomyModel->index(0,0);
    QStringList typelist{"layer","connection"};
    foreach (QString value, typelist)
        objTaxonomyModel->appendValue(value,parentIndex);
    // subtype layer
    parentIndex = parentIndex.child(0,0);
    typelist = QStringList{"iac_layer","bias_layer","custom"};
    foreach (QString value, typelist)
        objTaxonomyModel->appendValue(value,parentIndex);
    // subtype connection
    parentIndex = parentIndex.sibling(1,0);
    typelist = QStringList{"connection","bias","custom"};
    foreach (QString value, typelist)
        objTaxonomyModel->appendValue(value,parentIndex);

*/

    lazyNutObjView = new QTableView;
    //lazyNutObjView->horizontalHeader()->hide();
    lazyNutObjView->horizontalHeader()->setStretchLastSection(true);
    lazyNutObjView->setSelectionMode(QAbstractItemView::SingleSelection);

    objTaxonomyView = new QTreeView;
    objTaxonomyView->setModel(objTaxonomyModel);
    objTaxonomyView->setSelectionMode(QAbstractItemView::SingleSelection);


    connect(objTaxonomyView,SIGNAL(clicked(QModelIndex)),
            objTaxonomyModel,SLOT(getGenealogy(QModelIndex)));

    lazyNutObjTableModel = new LazyNutObjTableModel(objHash,this);


    connect(this,SIGNAL(beginObjHashModified()),lazyNutObjTableModel,SLOT(sendBeginResetModel()));
    connect(this,SIGNAL(endObjHashModified()),lazyNutObjTableModel,SLOT(sendEndResetModel()));




    lazyNutObjTableView = new QTableView;
    lazyNutObjTableView->setModel(lazyNutObjTableModel);

    lazyNutObjTableProxyModel = new LazyNutObjTableProxyModel(this);
    lazyNutObjTableProxyModel->setDynamicSortFilter(true);
    //lazyNutObjTableProxyModel->setFilterKeyColumns(QList<int>({1}));
    //lazyNutObjTableProxyModel->addFilterFixedString(1,"layer");
    lazyNutObjTableProxyModel->setSourceModel(lazyNutObjTableModel);

    connect(objTaxonomyModel,SIGNAL(sendGenealogy(QList<QVariant>)),
            lazyNutObjTableProxyModel,SLOT(setFilterFromGenealogy(QList<QVariant>)));

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


    //layout = new QHBoxLayout;

    //splitter = new QSplitter(this);
    //layout->addWidget(objTaxonomyView);
    addWidget(objTaxonomyView);
    //splitter->addWidget(lazyNutObjTableView);
    //layout->addWidget(lazyNutObjTableProxyView);
    addWidget(lazyNutObjTableProxyView);
    //layout->addWidget(lazyNutObjView);
    addWidget(lazyNutObjView);
    //setLayout(layout);
    //setCentralWidget(splitter);
    lazyNutObjModel = new LazyNutObjModel(new LazyNutObj,objHash,this);


    connect(lazyNutObjTableProxyModel,SIGNAL(getObj(const QModelIndex&)),
            lazyNutObjModel, SLOT(getObjFromCatalogueIndex(const QModelIndex&)));
    connect(lazyNutObjModel, SIGNAL(showObj(LazyNutObj*, LazyNutObjCatalogue*)),
            this, SLOT(setObj(LazyNutObj*, LazyNutObjCatalogue*)));\

 //   setObj((*objHash)["layerA"],objHash);

    setWindowTitle(tr("Object Explorer"));
}

void ObjExplorer::setObj(LazyNutObj* obj, LazyNutObjCatalogue *objHash)
{
    delete lazyNutObjModel;
    lazyNutObjModel = new LazyNutObjModel(obj,objHash,this);
    comboBoxDelegate = new ComboBoxDelegate(objHash,this);
    lazyNutObjView->setModel(lazyNutObjModel);

    foreach(int row, obj->rowsWithStringList())
    {
        lazyNutObjView->setItemDelegateForRow(row,comboBoxDelegate);
        lazyNutObjView->openPersistentEditor(lazyNutObjModel->index(row));
    }

    lazyNutObjView->resizeRowsToContents();

    connect(comboBoxDelegate,SIGNAL(activatedDelegateIndex(const QModelIndex&,int)),
            lazyNutObjModel, SLOT(getObjFromDescriptionIndex(const QModelIndex,int)));
    connect(lazyNutObjView, SIGNAL(clicked(const QModelIndex&)),
            lazyNutObjModel, SLOT(getObjFromDescriptionIndex(const QModelIndex&)));
    connect(lazyNutObjTableProxyModel,SIGNAL(getObj(const QModelIndex&)),
            lazyNutObjModel, SLOT(getObjFromCatalogueIndex(const QModelIndex&)));
    connect(lazyNutObjModel, SIGNAL(showObj(LazyNutObj*, LazyNutObjCatalogue*)),
            this, SLOT(setObj(LazyNutObj*, LazyNutObjCatalogue*)));\

    emit (objSelected(objHash->key(obj)));

}
