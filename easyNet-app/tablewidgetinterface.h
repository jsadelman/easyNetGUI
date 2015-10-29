#ifndef TABLEWIDGETINTERFACE_H
#define TABLEWIDGETINTERFACE_H

#include <QWidget>
#include <QMap>

class ObjectCacheFilter;
class QDomDocument;
class QAbstractItemModel;
class DataFrameModel;
class ObjectUpdater;
class TrialDataFrameModel;

class TableWidgetInterface : public QWidget
{
    Q_OBJECT
public:
    TableWidgetInterface(QWidget *parent);
    virtual ~TableWidgetInterface();
    virtual QString currentTable()=0;
    void addHeaderReplaceRules(Qt::Orientation orientation, QString from, QString to);



public slots:

    void addTable(QString name);
    void updateTable(QDomDocument *domDoc, QString cmd);
    void deleteTable(QString name);
    void prepareToUpdateTable(QDomDocument *domDoc, QString cmd);
    void setPrettyHeaderFromJob();
    virtual void setCurrentTable(QString name)=0;

signals:
    void currentTableChanged(QString);
    void tableDeleted(QString);

protected slots:
    //

protected:
    virtual void addTable_impl(QString name)=0;
    virtual void updateTable_impl(QAbstractItemModel *model)=0;
    virtual void deleteTable_impl(QString name)=0;
    DataFrameModel * getDataFrameModel(QAbstractItemModel *model);




    QString nameFromCmd(QString cmd);
    ObjectCacheFilter *dataframeFilter;
    ObjectUpdater *dataframeUpdater;
    DataFrameModel *lastModel;
    QString lastName;
    QMap<QString, QAbstractItemModel*> modelMap;
    QMap<Qt::Orientation , QList<QPair<QString, QString> > > headerReplaceRules;


};

#endif // TABLEWIDGETINTERFACE_H
