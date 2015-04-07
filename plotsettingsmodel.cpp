#include "plotsettingsmodel.h"
#include <QDomDocument>

PlotSettingsModel::PlotSettingsModel(QDomDocument *plotSettingsXML, QObject *parent) :
    plotSettingsXML(plotSettingsXML), QAbstractListModel(parent)
{
    rolesMap[Qt::DisplayRole] = "value";
    rolesMap[PlotSettingsTypeRole] = "type";
    rolesMap[PlotSettingsCommentRole] = "comment";
    rolesMap[PlotSettingsDefaultRole] = "default";
}

PlotSettingsModel::~PlotSettingsModel()
{
    delete plotSettingsXML;
}

void PlotSettingsModel::setXML(QDomDocument *domDoc)
{
    emit beginResetModel();
    delete plotSettingsXML;
    plotSettingsXML = domDoc;
    emit endResetModel();
}

int PlotSettingsModel::rowCount(const QModelIndex &/*parent*/) const
{
    return plotSettingsXML->firstChild().childNodes().size();
}

QVariant PlotSettingsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QDomNode settingNode = plotSettingsXML->firstChild().childNodes().at(index.row());

    if (role == PlotSettingsNameRole)
        return settingNode.toElement().attribute("label");

    else
    {
        QDomNode labelValueNode = settingNode.firstChild();
        QString label;
        while (!labelValueNode.isNull())
        {
            label = labelValueNode.toElement().attribute("label");
            if (label == rolesMap[role])
                return labelValueNode.toElement().attribute("value");

            labelValueNode = labelValueNode.nextSibling();
        }
        return QVariant(); // should not be reached
    }
}

Qt::ItemFlags PlotSettingsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
             return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool PlotSettingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        QDomNode labelValueNode = plotSettingsXML->firstChild().childNodes().at(index.row()).firstChild();
        QString label;
        while (!labelValueNode.isNull())
        {
            label = labelValueNode.toElement().attribute("label");
            if (label == rolesMap[role])
            {
                labelValueNode.toElement().setAttribute("value", value.toString());
                return true;
            }
            labelValueNode = labelValueNode.nextSibling();
        }
    }
    return false;
}
