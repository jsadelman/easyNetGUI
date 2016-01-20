#ifndef DATAFRAMEVIEWER_H
#define DATAFRAMEVIEWER_H

#include "dataviewer.h"

#include <QMap>
#include <QMultiMap>
#include <QTextDocument>

class DataFrameModel;
class PrettyHeadersModel;
class QTableView;
class ObjectCacheFilter;
class ObjectUpdater;
class QDomDocument;
class FindDialog;

class DataframeViewer : public DataViewer
{
    Q_OBJECT
    Q_PROPERTY(bool dragDropColumns READ dragDropColumns WRITE setDragDropColumns NOTIFY dragDropColumnsChanged)
    Q_PROPERTY(bool stimulusSet READ stimulusSet WRITE setStimulusSet NOTIFY stimulusSetChanged)
    Q_PROPERTY(bool parametersTable READ parametersTable WRITE setParametersTable NOTIFY parametersTableChanged)
public:
    DataframeViewer(Ui_DataViewer *ui, QWidget * parent = 0);
    virtual bool contains(QString name) Q_DECL_OVERRIDE;
    bool dragDropColumns() {return m_dragDropColumns;}
    void setDragDropColumns(bool enabled) {m_dragDropColumns = enabled; emit dragDropColumnsChanged(enabled);}
    bool stimulusSet() {return m_stimulusSet;}
    void setStimulusSet(bool isStimulusSet) {m_stimulusSet = isStimulusSet; emit stimulusSetChanged(isStimulusSet);}
    bool parametersTable() {return m_parametersTable;}
    void setParametersTable(bool isParametersTable) {m_parametersTable = isParametersTable; emit parametersTableChanged(isParametersTable);}


public slots:
    virtual void addItem(QString name="", bool setCurrent=false) Q_DECL_OVERRIDE;
    void setPrettyHeadersForTrial(QString trial, QString df);
    virtual void dispatch();
    virtual void open() Q_DECL_OVERRIDE;
    virtual void save() Q_DECL_OVERRIDE;
    virtual void copy() Q_DECL_OVERRIDE;
    void copyDataframe();
    void dataframeMerge();

protected slots:
    virtual void initiateRemoveItem(QString name) Q_DECL_OVERRIDE;
    virtual void removeItem(QString name) Q_DECL_OVERRIDE;
    virtual void enableActions(bool enable) Q_DECL_OVERRIDE;
    virtual void updateCurrentItem(QString name) Q_DECL_OVERRIDE;
    void updateDataframe(QDomDocument* domDoc, QString name);
    void showFindDialog();
    void findForward(const QString &str, QFlags<QTextDocument::FindFlag> flags);
    void setParameter(QString name, QString key_val);

signals:
    void dragDropColumnsChanged(bool);
    void stimulusSetChanged(bool);
    void parametersTableChanged(bool);

protected:

    QMap<QString, DataFrameModel*> modelMap;
    QMultiMap<QString, QTableView*> viewsMap;
    QMap<QString, PrettyHeadersModel*> prettyHeadersModelMap;
    ObjectCacheFilter *dataframeFilter;
    ObjectUpdater *dataframeUpdater;
    bool m_dragDropColumns;
    bool m_stimulusSet;
    bool m_parametersTable;
    FindDialog*     findDialog;

};

#endif // DATAFRAMEVIEWER_H
