#ifndef COMBOTABLEWIDGET_H
#define COMBOTABLEWIDGET_H

#include "tablewidgetinterface.h"

class QComboBox;
class QTableView;


class ComboTableWidget : public TableWidgetInterface
{
    Q_OBJECT
public:
    ComboTableWidget(QWidget *parent);
     virtual QString currentTable() Q_DECL_OVERRIDE;

public slots:
    virtual void setCurrentTable(QString name) Q_DECL_OVERRIDE;



protected:
    virtual void updateTable_impl(QAbstractItemModel *model) Q_DECL_OVERRIDE;
    virtual void deleteTable_impl(QString name) Q_DECL_OVERRIDE;
    virtual void setFilter_impl() Q_DECL_OVERRIDE {}

private slots:
    void setModel(QString name);

private:
    void buildWidget();


    QComboBox *comboBox;
    QTableView *view;
};

#endif // COMBOTABLEWIDGET_H
