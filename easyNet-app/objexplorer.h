#ifndef TOYVIEW_H
#define TOYVIEW_H

#include <QString>
#include <QMap>
#include <QHash>
#include <QDialog>
#include <QMainWindow>
#include <QWidget>
#include <QStyledItemDelegate>
#include <QLabel>

#include "treemodel.h"
#include "enumclasses.h"
#include "lazynutjob.h"
#include "sessionmanager.h"
class ObjectCache;
class AsLazyNutObject;
class QDomDocument;
class ExpandToFillButton;
class ObjectUpdater;
//typedef QHash<QString,LazyNutObject*> LazyNutObjectCatalogue;

class LazyNutObjectModel;
class LazyNutObjectListModel;
class ObjectCacheFilter;
class LazyNutObjTableModel;
//class LazyNutObjTableProxyModel;
class QSortFilterProxyModel;
class QListView;
class QTreeView;
class QColumnView;
class QListWidget;
//class QSplitter;
class QGroupBox;
class QVBoxLayout;
class ComboBoxDelegate;
class QStackedWidget;
class QSplitter;

struct DomItem;

class ObjExplorer: public QMainWindow
{
    Q_OBJECT

public:
    ObjExplorer(ObjectCache *objectCache, QWidget *parent = 0);
// should be private:


//    QTreeView *objTaxonomyView;
    //QComboBox *cbox;
//    ComboBoxDelegate *comboBoxDelegate;

signals:
//    void beginObjHashModified();
//    void endObjHashModified();
    void objectSelected(QString);
    void updateDiagramScene();

public slots:
//    void setObj(LazyNutObj* obj, LazyNutObjCatalogue *objectCatalogue);
//    void setObjFromListIndex(QModelIndex index);
//    void setObjFromObjName(QString name);
//    void updateLazyNutObjCatalogue(QDomDocument *domDoc);
//    void setTaxonomyView();
//    void reloadTaxonomy();
//    void resetLazyNutObjTableModel();

private slots:
    void queryTypes();
    void initTypes(QStringList types);
    void selectType(QString type);
//    void getTaxonomySubtypes();
//    void setTaxonomySubtypes(QStringList subtypes, QString cmd);
//    void connectTaxonomyModel();
    void triggerFillList(QAbstractItemModel*,const QModelIndex&at,QString cmd);
public slots:
    void doFillList(QAbstractItemModel*,QDomDocument*dom,const QModelIndex&at);
//    void setObjFromProxyTableIndex(QModelIndex index);


private:
    ObjectCache  *objectCache;
    //---------- Type list ---------//
    QListWidget *typeList;
    QString allObjectsString;
    //---------- Object list ---------//
    ObjectCacheFilter *objectListFilter;
    QListView *objectListView;
    //--------- Description ----------//
    ObjectCacheFilter *descriptionFilter;
    ObjectUpdater *descriptionUpdater;
    LazyNutObjectModel *objectModel;
    ExpandToFillButton *expandToFillButton;
    QTreeView *objectView;
    QSplitter *splitter;




};
struct localListFiller:QObject
{
   Q_OBJECT
public:
   localListFiller(QAbstractItemModel*qaim,ObjExplorer* oe,const QModelIndex&at,QString cmd):qaim_(qaim),oe_(oe),at_(at)
   {
       LazyNutJob *job = new LazyNutJob;
       job->cmdList = QStringList({QString("xml %1").arg(cmd)});
       job->setAnswerReceiver(this, SLOT(doFillList(QDomDocument*)), AnswerFormatterType::XML);
       SessionManager::instance()->submitJobs(job);
   }

   virtual ~localListFiller();
private slots:
   void doFillList(QDomDocument*dom)
   {
       oe_->doFillList(qaim_,dom,at_);
   }

private:
   ObjExplorer* oe_;
   QModelIndex at_;
   QAbstractItemModel* qaim_;
};

#endif // TOYVIEW_H
