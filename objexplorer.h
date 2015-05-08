#ifndef TOYVIEW_H
#define TOYVIEW_H

#include <QString>
#include <QMap>
#include <QHash>
#include <QDialog>
#include <QMainWindow>
#include <QWidget>
#include <QStyledItemDelegate>

#include "treemodel.h"

class LazyNutObj;
class LazyNutObject;
class QDomDocument;
typedef QHash<QString,LazyNutObject*> LazyNutObjectCatalogue;

class LazyNutObjModel;
class LazyNutObjectListModel;
class LazyNutObjTableModel;
class LazyNutObjTableProxyModel;
class QListView;
class QTableView;
class QTreeView;
class QColumnView;
//class QSplitter;
class QGroupBox;
class QVBoxLayout;
class ComboBoxDelegate;
class QStackedWidget;


class ObjExplorer: public QMainWindow //QSplitter //QWidget
{
    Q_OBJECT

public:
    ObjExplorer(LazyNutObjectCatalogue *objectCatalogue, QWidget *parent = 0);
// should be private:


    LazyNutObjectCatalogue  *objectCatalogue;
    LazyNutObject *lazyNutObject;

    LazyNutObjModel *lazyNutObjModel;
    TreeModel *objTaxonomyModel;
    LazyNutObjectListModel *lazyNutObjectListModel;
    LazyNutObjTableModel *lazyNutObjTableModel;
    LazyNutObjTableProxyModel *lazyNutObjTableProxyModel;

    QListView *lazyNutObjectListView;
//    QVBoxLayout *layout;
    QTableView *lazyNutObjTableView;
    QTableView *lazyNutObjTableProxyView;
    QTreeView *lazyNutObjectView;
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
    void setObjFromListIndex(QModelIndex index);
    void setObjFromObjName(QString name);
    void updateLazyNutObjCatalogue(QDomDocument *domDoc);
    void setTaxonomyView();
    void reloadTaxonomy();
    void resetLazyNutObjTableModel();

private slots:
    void getTaxonomyTypes();
    void setTaxonomyTypes(QStringList types);
    void getTaxonomySubtypes();
    void setTaxonomySubtypes(QStringList subtypes, QString cmd);
    void connectTaxonomyModel();



private:
    void createTaxonomy();
    void createTaxonomyMenu();

     QListView *taxListView;
     QColumnView *taxColumnView;
     QTreeView *taxTreeView;
     QStackedWidget *taxWidget;

    QMap<QString, QWidget*> taxViewMap;
    QMenu *taxonomyMenu;

};

#endif // TOYVIEW_H
