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
typedef QHash<QString,LazyNutObj*> LazyNutObjCatalogue;
class LazyNutObjModel;
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
    ObjExplorer(LazyNutObjCatalogue* objHash, TreeModel* objTaxonomyModel, QWidget *parent = 0);
// should be private:

    LazyNutObjCatalogue  *objHash;


    LazyNutObjModel *lazyNutObjModel;
    TreeModel *objTaxonomyModel;
    LazyNutObjTableModel *lazyNutObjTableModel;
    LazyNutObjTableProxyModel *lazyNutObjTableProxyModel;

    QSplitter *splitter;
//    QVBoxLayout *layout;
    QTableView *lazyNutObjTableView;
    QTableView *lazyNutObjTableProxyView;
    QTableView *lazyNutObjView;
    QTreeView *objTaxonomyView;
    //QComboBox *cbox;
    ComboBoxDelegate *comboBoxDelegate;

signals:
    void beginObjHashModified();
    void endObjHashModified();
    void objSelected(QString);

public slots:
    void setObj(LazyNutObj* obj, LazyNutObjCatalogue *objHash);

};

#endif // TOYVIEW_H
