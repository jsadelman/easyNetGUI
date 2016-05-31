#ifndef TOYVIEW_H
#define TOYVIEW_H

#include <QMainWindow>

class ObjectCache;
class QDomDocument;
class ObjectUpdater;
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


class ObjExplorer: public QMainWindow
{
    Q_OBJECT

public:
    ObjExplorer(QWidget *parent = 0);

signals:
    void objectSelected(QString);
    void updateDiagramScene();

private slots:
    void queryTypes();
    void initTypes(QStringList types);
    void selectType(QString type);


private:
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
    QTreeView *objectView;
    QSplitter *splitter;

};


#endif // TOYVIEW_H
