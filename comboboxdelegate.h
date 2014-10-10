#include <QStyledItemDelegate>
#include <QComboBox>
#include "lazynutobj.h"

class QPersistentModelIndex;

class ComboBoxInDelegate: public QComboBox
{
    Q_OBJECT

public:
    ComboBoxInDelegate(const QModelIndex &delegateModelIndex, QWidget *parent = 0);

    const QPersistentModelIndex delegateModelIndex;
    int delegateRow;

public slots:
    void associateDelegateIndex(int objIndex);

signals:
    void activatedDelegateIndex(const QModelIndex &delegateModelIndex, int objIndex);
};



class ComboBoxDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    ComboBoxDelegate(LazyNutObjCatalogue *objHash, QObject *parent = 0):
        QStyledItemDelegate(parent), _objHash(objHash) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

// public slots:
 //   void getSelectedObj(QModelIndex &delegateModelIndex, int objIndex);

signals:
    void activatedDelegateIndex(const QModelIndex &delegateModelIndex, int index);

private:
    LazyNutObjCatalogue *_objHash;

};
