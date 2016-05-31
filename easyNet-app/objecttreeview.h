#ifndef OBJECTTREEVIEW_H
#define OBJECTTREEVIEW_H

#include <QTreeView>
#include <QModelIndex>
#include "sessionmanager.h"
#include "lazynutjob.h"

class LazyNutObjectModel;
class ExpandToFillButton;
class QDomDocument;


class ObjectTreeView : public QTreeView
{
    Q_OBJECT
public:
    ObjectTreeView(QWidget *parent);
    virtual void setModel(QAbstractItemModel *_objectModel) Q_DECL_OVERRIDE;
    virtual QSize minimumSizeHint() const Q_DECL_OVERRIDE;

public slots:
     void doFillList(QDomDocument*dom);
     void triggerFillList(QAbstractItemModel*,const QModelIndex& ind, QString name) ;

private:
    LazyNutObjectModel *objectModel;
    ExpandToFillButton *expandToFillButton;
    QModelIndex index_to_expand;
};


#endif // OBJECTTREEVIEW_H
