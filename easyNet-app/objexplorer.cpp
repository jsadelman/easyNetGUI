#include <QComboBox>
#include <QStringList>
#include <QListView>
#include <QListWidget>
#include <QSplitter>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>


#include "objexplorer.h"
#include "sessionmanager.h"
#include "lazynutjob.h"
#include "expandtofillbutton.h"
#include "objectcache.h"
#include "objectcachefilter.h"

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

    connect(objectListView->selectionModel(), &QItemSelectionModel::currentChanged, [=](const QModelIndex & index)
    {
        emit objectSelected(objectListFilter->data(index).toString());
    });


    //------- splitter ----------//

    splitter = new QSplitter;
    splitter->setOrientation(Qt::Horizontal);
    splitter->addWidget(typeList);
    splitter->addWidget(objectListView);
//    splitter->addWidget(objectView);

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
