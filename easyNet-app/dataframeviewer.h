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
class QToolButton;

class DataframeViewer : public DataViewer
{
    Q_OBJECT
    friend class DataframeViewerDispatcher;
    Q_PROPERTY(bool dragDropColumns READ dragDropColumns WRITE setDragDropColumns NOTIFY dragDropColumnsChanged)
    Q_PROPERTY(bool stimulusSet READ stimulusSet WRITE setStimulusSet NOTIFY stimulusSetChanged)
    Q_PROPERTY(bool parametersTable READ parametersTable WRITE setParametersTable NOTIFY parametersTableChanged)
public:
    DataframeViewer(Ui_DataViewer *ui, QWidget * parent = 0);
    bool dragDropColumns() {return m_dragDropColumns;}
    void setDragDropColumns(bool enabled) {m_dragDropColumns = enabled; emit dragDropColumnsChanged(enabled);}
    bool stimulusSet() {return m_stimulusSet;}
    void setStimulusSet(bool isStimulusSet) {m_stimulusSet = isStimulusSet; emit stimulusSetChanged(isStimulusSet);}
    bool parametersTable() {return m_parametersTable;}
    void setParametersTable(bool isParametersTable);

public slots:
    void setPrettyHeadersForTrial(QString trial, QString df);
    virtual void dispatch() Q_DECL_OVERRIDE;
    virtual void open() Q_DECL_OVERRIDE;
    virtual void save() Q_DECL_OVERRIDE;
    virtual void copy() Q_DECL_OVERRIDE;
    void copyDataframe();
//    void dataframeMerge();
    void addRProcessedDataframe(QString name, bool setCurrent=true, bool isBackup=false, QList<QSharedPointer<QDomDocument> > info=QList<QSharedPointer<QDomDocument> >());

protected slots:
    virtual void destroyItem_impl(QString name) Q_DECL_OVERRIDE;
    virtual void enableActions(bool enable) Q_DECL_OVERRIDE;
    virtual void setCurrentItem(QString name) Q_DECL_OVERRIDE;
    void updateDataframe(QDomDocument* domDoc, QString name);
    void showFindDialog();
    void findForward(const QString &str, QFlags<QTextDocument::FindFlag> flags);
    void setParameter(QString name, QString key_val);
    void sendNewPlotRequest();
    void sendNewDataframeViewRequest();

signals:
    void dragDropColumnsChanged(bool);
    void stimulusSetChanged(bool);
    void parametersTableChanged(bool);
    void newPlotRequested(QString name, QString type, QMap<QString, QString> defaultSettings,
                       int flags, QList<QSharedPointer<QDomDocument> > info);
    void newDataframeViewRequested(QString name, QString type, QMap<QString, QString> defaultSettings,
                       int flags, QList<QSharedPointer<QDomDocument> > info);

protected:
    virtual void addItem_impl(QString name) Q_DECL_OVERRIDE;
    virtual QWidget *makeView(QString name);
    virtual void addNameToFilter(QString name);
    virtual void removeNameFromFilter(QString name);
    virtual void setNameInFilter(QString name);
    void addExtraActions();


    QMap<QString, DataFrameModel*> modelMap;
    QMap<QString, PrettyHeadersModel*> prettyHeadersModelMap;
    ObjectCacheFilter *dataframeFilter;
    ObjectUpdater *dataframeUpdater;
//    ObjectCacheFilter *dataframeDescriptionFilter;
//    ObjectUpdater *dataframeDescriptionUpdater;
    QStringList requestedDataframeViews;
    bool m_dragDropColumns;
    bool m_stimulusSet;
    bool m_parametersTable;
    FindDialog*     findDialog;
    QAction *findAct;
    QAction *copyDFAct;
//    QAction *dataframeMergeAct;
    QToolButton *plotButton;
    QToolButton *dataframeViewButton;

};

#endif // DATAFRAMEVIEWER_H
