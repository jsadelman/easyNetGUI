#ifndef PARAMETERSMODEL_H
#define PARAMETERSMODEL_H

#include "dataframemodel.h"

class ParametersModel : public DataFrameModel
{
    Q_OBJECT
public:
    ParametersModel(QDomDocument *domDoc, QObject *parent=0);
    bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE;
    QString value(QString parameter);

signals:
    void newParamValueSig(QString,QString);

protected:
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

};

#endif // PARAMETERSMODEL_H
