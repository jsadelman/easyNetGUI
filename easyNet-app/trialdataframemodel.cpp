#include "trialdataframemodel.h"
#include "dataframemodel.h"
#include <QDebug>

TrialDataFrameModel::TrialDataFrameModel(QObject *parent)
    : QIdentityProxyModel(parent)
{
    headerReplace[Qt::Horizontal] = QList<QPair<QString, QString> > ();
    headerReplace[Qt::Vertical] = QList<QPair<QString, QString> > ();
}

QVariant TrialDataFrameModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QIdentityProxyModel::headerData(section, orientation, role);

    DataFrameModel *dfModel = qobject_cast<DataFrameModel *>(sourceModel());
    if (dfModel)
    {
        QString header = dfModel->headerData(section, orientation, role).toString();
        QPair<QString, QString> replacePair;
        foreach(replacePair, headerReplace.value(orientation))
            header.replace(QRegExp(replacePair.first), replacePair.second);

        return header;
    }
    else
        return QIdentityProxyModel::headerData(section, orientation, role);
}

void TrialDataFrameModel::addHeaderReplace(Qt::Orientation orientation, QString from, QString to)
{
    headerReplace[orientation].append(QPair<QString, QString>(from, to));
}
