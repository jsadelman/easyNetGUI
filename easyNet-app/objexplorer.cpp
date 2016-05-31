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
#include "sessionmanager.h"
#include "lazynutjob.h"
#include "xmlelement.h"
#include "expandtofillbutton.h"
#include "objectcache.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "domitem.h"

ObjExplorer::ObjExplorer(QWidget *parent)
    : QMainWindow(parent)
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
    descriptionUpdater = new ObjectUpdater(this);
    descriptionUpdater->setProxyModel(descriptionFilter);

    objectModel = new LazyNutObjectModel(nullptr, this);
    connect(descriptionFilter, SIGNAL(objectCreated(QString, QString, QString, QDomDocument*)),
            objectModel, SLOT(setDescription(QString, QString, QString, QDomDocument*)));
    connect(descriptionFilter, SIGNAL(objectDestroyed(QString)),
            objectModel, SLOT(removeDescription(QString)));
    connect(descriptionUpdater, SIGNAL(objectUpdated(QDomDocument*, QString)),
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
    connect(objectModel, &LazyNutObjectModel::objectRequested, [=](QString name)
    {
        SessionManager::instance()->descriptionCache->create(name);
    });
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


void ObjExplorer::queryTypes()
{
    LazyNutJob *job = new LazyNutJob;
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
