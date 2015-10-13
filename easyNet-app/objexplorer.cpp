#include <QtWidgets>
#include <QtGlobal>
#include <QtDebug>
#include <QComboBox>
#include <QStringList>
#include <QListView>
#include <QTreeView>
#include <QSplitter>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QItemSelectionModel>


#include "objexplorer.h"
#include "lazynutobject.h"
#include "lazynutobjectmodel.h"
#include "lazynutobjtablemodel.h"
#include "sessionmanager.h"
#include "lazynutjob.h"
#include "xmlelement.h"
#include "expandtofillbutton.h"
#include "lazynutlistwidget.h"
#include "objectcache.h"
#include "objectcataloguefilter.h"
#include "descriptionupdater.h"
#include "domitem.h"

ObjExplorer::ObjExplorer(ObjectCache *objectCache, QWidget *parent)
    : objectCache(objectCache), QMainWindow(parent)
{

    //---------- Type list ---------//

    typeList = new QListWidget(this);
    typeList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    typeList->setSelectionMode(QAbstractItemView::SingleSelection);
    allObjectsString = "<all objects>";
    connect(SessionManager::instance(), SIGNAL(lazyNutStarted()), this, SLOT(queryTypes()));

    //---------- Object list ---------//

    objectListFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);

    connect(typeList, SIGNAL(currentTextChanged(QString)), this, SLOT(selectType(QString)));



    objectListView = new QListView(this);
    objectListView->setModel(objectListFilter);
    objectListView->setModelColumn(0);
    objectListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    objectListView->setSelectionMode(QAbstractItemView::SingleSelection);

    //--------- Description ----------//

    descriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);

    connect(objectListView->selectionModel(), &QItemSelectionModel::currentChanged, [=](const QModelIndex & index)
    {
        descriptionFilter->setName(objectListFilter->data(index).toString());
    });

    descriptionFilter->setName("<no name>");
    descriptionUpdater = new DescriptionUpdater(this);
    descriptionUpdater->setProxyModel(descriptionFilter);

    objectModel = new LazyNutObjectModel(nullptr, this);
    connect(descriptionFilter, SIGNAL(objectCreated(QString, QString, QDomDocument*)),
            objectModel, SLOT(setDescription(QString, QString, QDomDocument*)));
    connect(descriptionFilter, SIGNAL(objectDestroyed(QString)),
            objectModel, SLOT(removeDescription(QString)));
    connect(descriptionUpdater, SIGNAL(descriptionUpdated(QDomDocument*)),
            objectModel, SLOT(updateDescription(QDomDocument*)));



    objectView = new QTreeView;
    objectView->setModel(objectModel);
    objectView->header()->setStretchLastSection(true);
    objectView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    objectView->setSelectionMode(QAbstractItemView::SingleSelection);
    expandToFillButton = new ExpandToFillButton(objectView);
    objectView->setItemDelegateForColumn(1, expandToFillButton);
    connect(expandToFillButton, SIGNAL(expandToFill(QAbstractItemModel*,const QModelIndex&,QString)),
              this, SLOT(triggerFillList(QAbstractItemModel*,const QModelIndex&,QString)));
    connect(objectModel, &QAbstractItemModel::modelReset, [=]()
    {
        objectView->expandAll();
        //objectView->expand(objectModel->index(0,0));
        for (int col=0; col < objectView->model()->columnCount(); ++col)
            objectView->resizeColumnToContents(col);
    });
    connect(objectView, SIGNAL(clicked(QModelIndex)), objectModel, SLOT(sendObjectRequested(QModelIndex)));
    connect(objectModel, SIGNAL(objectRequested(QString)), descriptionFilter, SLOT(setName(QString)));
    connect(this, SIGNAL(objectSelected(QString)), descriptionFilter, SLOT(setName(QString)));

    //------- splitter ----------//

    splitter = new QSplitter;
    splitter->setOrientation(Qt::Horizontal);
    splitter->addWidget(typeList);
    splitter->addWidget(objectListView);
    splitter->addWidget(objectView);

    setWindowTitle(tr("Object Explorer"));
    setCentralWidget(splitter);
    QList<int> list;
    list = splitter->sizes();
//    qDebug() << "***old sizes = " << list;
    list.clear();
    list.append(1);
    list.append(1);
    list.append(2);
    splitter->setSizes(list);
    list = splitter->sizes();
//    qDebug() << "***new sizes = " << list;
    list.clear();
    list.append(100);
    list.append(200);
    list.append(350);
//    qDebug() << "***attempted sizes = " << list;
    splitter->setSizes(list);
    list = splitter->sizes();
//    qDebug() << "***new sizes = " << list;
//    splitter->setStretchFactor(0, 1);
//    splitter->setStretchFactor(1, 1);
//    splitter->setStretchFactor(2, 2);

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

//void ObjExplorer::setObjFromListIndex(QModelIndex index)
//{
////    setObjFromObjName(lazyNutObjectListModel->data(index,Qt::DisplayRole).toString());
//    setObjFromObjName(lazyNutObjTableModel->data(index,Qt::DisplayRole).toString());

//}

//void ObjExplorer::setObjFromObjName(QString name)
//{
//    AsLazyNutObject *lno = objectCatalogue->value(name);
//    if (!lno)
//        return;
////    if (lazyNutObjectModel)
////        delete lazyNutObjectModel;

//    objectModel = new LazyNutObjectModel(lno);
//    QAbstractItemModel *oldModel = objectView->model();
//    QItemSelectionModel *oldSelectionModel = objectView->selectionModel();
//    objectView->setModel(objectModel);
//    if (oldSelectionModel)
//    {
////        oldModel->deleteLater();
//        oldSelectionModel->deleteLater();
//    }
//    objectView->expandAll();
//    objectView->resizeColumnToContents(0);
//    objectView->resizeColumnToContents(1);
//    connect(objectView,SIGNAL(clicked(QModelIndex)),
//            objectModel,SLOT(getObjFromDescriptionIndex(QModelIndex)));
//    connect(objectModel,SIGNAL(objectRequested(QString)),
//            this,SLOT(setObjFromObjName(QString)));
//    emit objectSelected(name);
//}

//void ObjExplorer::updateLazyNutObjCatalogue(QDomDocument *domDoc)
//{
////    qDebug () << domDoc->toString();
//    AsLazyNutObject *newObj = new AsLazyNutObject(domDoc);
////    QString name = newObj->name();
////    lazyNutObjTableModel->sendBeginResetModel();
//    delete objectCatalogue->value(newObj->name);
//    objectCatalogue->insert(newObj->name,newObj);
////    connect(lazyNutObjectView,SIGNAL(clicked(QModelIndex)),
////            newObj,SLOT(getObjFromDescriptionIndex(QModelIndex)));
////    connect(newObj,SIGNAL(objectRequested(QString)),
////            this,SLOT(setObjFromObjName(QString)));
////    lazyNutObjTableModel->sendEndResetModel();
//    //emit objCatalogueChanged();
//}

//void ObjExplorer::createTaxonomy()
//{
//    objTaxonomyModel = new TreeModel(QStringList({"type"}));

//    taxListView = new QListView;
//    taxColumnView = new QColumnView;
//    taxTreeView = new QTreeView;
//    taxTreeView->setHeaderHidden(true);

//    taxViewMap["list"] = taxListView;
//    taxViewMap["column"] = taxColumnView;
//    taxViewMap["tree"] = taxTreeView;


//    taxWidget = new QStackedWidget;
//    foreach(QString view, taxViewMap.keys())
//    {
//        taxWidget->addWidget(taxViewMap[view]);
//        qobject_cast<QAbstractItemView*>(taxViewMap[view])->setSelectionMode(QAbstractItemView::SingleSelection);
//        qobject_cast<QAbstractItemView*>(taxViewMap[view])->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    }

//    createTaxonomyMenu();
//}

//void ObjExplorer::createTaxonomyMenu()
//{
//    taxonomyMenu = menuBar()->addMenu("Taxonomy");
//    taxonomyMenu->addAction("Reload", this, SLOT(reloadTaxonomy()));
//    QMenu *taxViewMenu = taxonomyMenu->addMenu("View mode");
//    foreach(QString view, taxViewMap.keys())
//       taxViewMenu->addAction(view, this, SLOT(setTaxonomyView()));

//}

//void ObjExplorer::setTaxonomyView()
//{
//    taxWidget->setCurrentWidget(taxViewMap.value(qobject_cast<QAction*>(sender())->text()));
//}

//void ObjExplorer::reloadTaxonomy()
//{
//    if (SessionManager::instance()->isOn())
//        getTypes();
//}

//void ObjExplorer::resetLazyNutObjTableModel()
//{
//    lazyNutObjTableModel->sendBeginResetModel();
//    lazyNutObjTableModel->sendEndResetModel();
//}

void ObjExplorer::queryTypes()
{
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    job->cmdList = QStringList({"xml list type"});
    job->setAnswerReceiver(this, SLOT(initTypes(QStringList)), AnswerFormatterType::ListOfValues);
    SessionManager::instance()->submitJobs(job);
}

void ObjExplorer::initTypes(QStringList types)
{
   typeList->clear(); // safety
   typeList->addItem(allObjectsString);
   typeList->addItems(types);
   selectType(allObjectsString);
}

void ObjExplorer::selectType(QString type)
{
    if (type == allObjectsString)
        objectListFilter->setNoFilter();
    else
        objectListFilter->setType(type);
}


void ObjExplorer::showList(QString cmd)
{
    LazyNutListWidget *listWidget = new LazyNutListWidget(cmd);
    listWidget->setAttribute(Qt::WA_DeleteOnClose, true);
    listWidget->show();
}

localListFiller::~localListFiller()
{
}

void ObjExplorer::triggerFillList(QAbstractItemModel*qaim,const QModelIndex&at,QString cmd)
{
    auto llf=new localListFiller(qaim,this,at,cmd);
}

void ObjExplorer::doFillList(QAbstractItemModel*qaim,QDomDocument* dom,const QModelIndex&at)
{
    auto lno=dynamic_cast<LazyNutObjectModel*>(qaim);
    if(!lno) qDebug()<<"failed to find a LazyNutObjectModel";
    else lno->pokeAdditionalDescription(at,dom);
}

//void ObjExplorer::setObjFromProxyTableIndex(QModelIndex index)
//{
//    setObjFromObjName(lazyNutObjectTableProxy->data(index,Qt::DisplayRole).toString());
//}
