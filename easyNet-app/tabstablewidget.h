#ifndef TABSTABLEWIDGET_H
#define TABSTABLEWIDGET_H

#include "tablewidgetinterface.h"

class QTabWidget;


class TabsTableWidget : public TableWidgetInterface
{
    Q_OBJECT
public:
    TabsTableWidget(QWidget *parent);
    virtual QString currentTable() Q_DECL_OVERRIDE;
    QString tableAt(int index);

public slots:
   virtual void setCurrentTable(QString name) Q_DECL_OVERRIDE;



protected:
    virtual void addTable_impl(QString name) Q_DECL_OVERRIDE;
    virtual void updateTable_impl(QAbstractItemModel *model) Q_DECL_OVERRIDE;
    virtual void deleteTable_impl(QString name) Q_DECL_OVERRIDE;

private:
    void buildWidget();
    QTabWidget *tabWidget;
};

#endif // TABSTABLEWIDGET_H
