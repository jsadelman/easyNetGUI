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
#include "enumclasses.h"

//class LazyNutObj;
class LazyNutObject;
class QDomDocument;
class ExpandToFillButton;
//typedef QHash<QString,LazyNutObject*> LazyNutObjectCatalogue;

class LazyNutObjectModel;
class LazyNutObjectListModel;
class LazyNutObjTableModel;
class LazyNutObjTableProxyModel;
class QSortFilterProxyModel;
class QListView;
class QTableView;
class QTreeView;
class QColumnView;
class QListWidget;
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
    LazyNutObjectModel *lazyNutObjectModel;

//    LazyNutObjModel *lazyNutObjModel;
    TreeModel *objTaxonomyModel;
    LazyNutObjectListModel *lazyNutObjectListModel;
    LazyNutObjTableModel *lazyNutObjTableModel;
    LazyNutObjTableProxyModel *lazyNutObjTableProxyModel;
    QSortFilterProxyModel *lazyNutObjectTableProxy;



//    QListView *lazyNutObjectListView;
//    QVBoxLayout *layout;
    QListView *lazyNutObjectListView;
    ExpandToFillButton *expandToFillButton;
    QTableView *lazyNutObjectTableProxyView;
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
//    void setTaxonomyView();
//    void reloadTaxonomy();
    void resetLazyNutObjTableModel();

private slots:
    void getTypes();
    void setTypes(QStringList types);
//    void getTaxonomySubtypes();
//    void setTaxonomySubtypes(QStringList subtypes, QString cmd);
//    void connectTaxonomyModel();
    void showList(QString cmd);
    void setObjFromProxyTableIndex(QModelIndex index);


private:
//    void createTaxonomy();
//    void createTaxonomyMenu();

    QListWidget *typeList;
    QListView *taxListView;
    QColumnView *taxColumnView;
    QTreeView *taxTreeView;
    QStackedWidget *taxWidget;

    QMap<QString, QWidget*> taxViewMap;
    QMenu *taxonomyMenu;

};

#endif // TOYVIEW_H
