#ifndef OBJECTNAVIGATOR_H
#define OBJECTNAVIGATOR_H

#include <QMainWindow>
#include <QModelIndex>
#include <QStack>


#include "sessionmanager.h"
#include "lazynutjob.h"


class QDomDocument;
class ExpandToFillButton;
class ObjectUpdater;
class ObjectCacheFilter;
class LazyNutObjectModel;
class ObjectTreeView;
class QAbstractItemModel;



class ObjectNavigator : public QMainWindow
{
    Q_OBJECT
public:
    explicit ObjectNavigator(QWidget *parent = 0);

public slots:
    void setObject(QString name);

private slots:
    void forward();
    void back();

private:
    ObjectCacheFilter *descriptionFilter;
    ObjectUpdater *descriptionUpdater;
    LazyNutObjectModel *objectModel;
    ObjectTreeView *objectView;
    QStack<QString> forwardStack;
    QStack<QString> backwardStack;
    QAction *forwardAct;
    QAction *backwardAct;
    QToolBar *navigationToolBar;
    QString currentObject;

};


#endif // OBJECTNAVIGATOR_H
