#include "prettyheadersmodel.h"
#include "dataframemodel.h"
#include <QDebug>

PrettyHeadersModel::PrettyHeadersModel(QObject *parent)
    : headerReplaceRules(), QIdentityProxyModel(parent)
{
//    headerReplace[Qt::Horizontal] = QList<QPair<QString, QString> > ();
//    headerReplace[Qt::Vertical] = QList<QPair<QString, QString> > ();
}

QVariant PrettyHeadersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QIdentityProxyModel::headerData(section, orientation, role);

    DataFrameModel *dfModel = qobject_cast<DataFrameModel *>(sourceModel());
    if (dfModel)
    {
        QString header = dfModel->headerData(section, orientation, role).toString();
        QPair<QString, QString> replacePair;
        foreach(replacePair, headerReplaceRules.value(orientation))
            header.replace(QRegExp(replacePair.first), replacePair.second);

        return header;
    }
    else
        return QIdentityProxyModel::headerData(section, orientation, role);
}

void PrettyHeadersModel::addHeaderReplaceRules(Qt::Orientation orientation, QString from, QString to)
{
    headerReplaceRules[orientation].append(QPair<QString, QString>(from, to));
}

void PrettyHeadersModel::setHeadeReplaceRules(QMap<Qt::Orientation, QList<QPair<QString, QString> > > rules)
{
    headerReplaceRules = rules;
}


QString PrettyHeadersModel::name()
{
    DataFrameModel *dfModel = qobject_cast<DataFrameModel *>(sourceModel());
    if (dfModel)
        return dfModel->name();
    else
        return QString();
}

QTableView *PrettyHeadersModel::view()
{
    DataFrameModel *dfModel = qobject_cast<DataFrameModel *>(sourceModel());
    if (dfModel)
        return dfModel->view();
    else
        return nullptr;
}
