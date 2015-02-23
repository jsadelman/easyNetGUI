#ifndef TOYVIEW_H
#define TOYVIEW_H

#include <QString>
#include <QMap>
#include <QHash>
#include <QDialog>
#include <QMainWindow>
#include <QWidget>
#include <QStyledItemDelegate>
#include <QSplitter>

#include "treemodel.h"

class LazyNutObj;
class LazyNutObject;
class QDomDocument;
//typedef QHash<QString,LazyNutObj*> LazyNutObjCatalogue;
typedef QHash<QString,LazyNutObject*> LazyNutObjectCatalogue;

class LazyNutObjModel;
class LazyNutObjectListModel;
class LazyNutObjTableModel;
class LazyNutObjTableProxyModel;
class QListView;
class QTableView;
class QTreeView;
//class QSplitter;
class QGroupBox;
class QVBoxLayout;
class ComboBoxDelegate;


class ObjExplorer: public QSplitter //QWidget
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
//    QTableView *lazyNutObjTableView;
//    QTableView *lazyNutObjTableProxyView;
    QTreeView *lazyNutObjectView;
//    QTreeView *objTaxonomyView;
    //QComboBox *cbox;
//    ComboBoxDelegate *comboBoxDelegate;

signals:
    void beginObjHashModified();
    void endObjHashModified();
    void objectSelected(QString);

public slots:
//    void setObj(LazyNutObj* obj, LazyNutObjCatalogue *objectCatalogue);
    void setObjFromListIndex(QModelIndex index);
    void setObjFromObjName(QString name);
    void updateLazyNutObjCatalogue(QDomDocument *domDoc);

};

#endif // TOYVIEW_H
