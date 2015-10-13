#ifndef TABLEWIDGETINTERFACE_H
#define TABLEWIDGETINTERFACE_H

#include <QWidget>
#include <QMap>

class ObjectCatalogueFilter;
class QDomDocument;
class QAbstractItemModel;
class DataFrameModel;

class TableWidgetInterface : public QWidget
{
    Q_OBJECT
public:
    TableWidgetInterface(QWidget *parent);
    virtual ~TableWidgetInterface();
    virtual QString currentTable()=0;

    void setFilter(ObjectCatalogueFilter *filter);
    ObjectCatalogueFilter * getFilter() {return m_filter;} // for direct manipulation

public slots:
    void updateTable(QDomDocument *domDoc, QString cmd);
    void deleteTable(QString name);
    void prepareToUpdateTable(QDomDocument *domDoc, QString cmd);
    void setPrettyHeaderFromJob();
    virtual void setCurrentTable(QString name)=0;



protected:
    virtual void updateTable_impl(QAbstractItemModel *model)=0;
    virtual void deleteTable_impl(QString name)=0;
    virtual void setFilter_impl()=0; // setup signals and slots
    DataFrameModel * getDataFrameModel(QAbstractItemModel *model);




    QString nameFromCmd(QString cmd);
    ObjectCatalogueFilter *m_filter;
    DataFrameModel *lastModel;
    QString lastName;
    QMap<QString, QAbstractItemModel*> modelMap;

};

#endif // TABLEWIDGETINTERFACE_H
