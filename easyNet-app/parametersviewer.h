#ifndef PARAMETERSVIEWER_H
#define PARAMETERSVIEWER_H

#include "dataframeviewer.h"

class ParametersViewer : public DataframeViewer
{
    Q_OBJECT
    friend class DataframeViewerDispatcher;
public:
    ParametersViewer(Ui_DataViewer *ui, QWidget * parent = 0);

protected slots:
    virtual void destroyItem_impl(QString name) Q_DECL_OVERRIDE;
    virtual void enableActions(bool enable) Q_DECL_OVERRIDE;
    virtual void updateDataframe(QDomDocument* domDoc, QString name) Q_DECL_OVERRIDE;
    void restoreSelection();
    void restoreAll();
    void paramExplore();

signals:
    void paramExploreDfCreated(QString);


protected:
    QString reference(QString name);

    QMap<QString, ParametersProxyModel*> referenceProxyMap;
    QAction *restoreSelectionAct;
    QAction *restoreAllAct;
    QToolButton *restoreButton;
    QAction *paramExploreAct;
    QMenu *restoreMenu;
};

#endif // PARAMETERSVIEWER_H
