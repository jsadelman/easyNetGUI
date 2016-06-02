#ifndef PARAMETERSPROXYMODEL_H
#define PARAMETERSPROXYMODEL_H


#include <QIdentityProxyModel>

class DataFrameModel;

class ParametersProxyModel : public QIdentityProxyModel
{
public:
    ParametersProxyModel(QObject *parent = nullptr);
    virtual void setSourceModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;
    void setReferenceModel(DataFrameModel *model);
    virtual QVariant data(const QModelIndex &ind, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    DataFrameModel *m_referenceModel;
};

#endif // PARAMETERSPROXYMODEL_H
