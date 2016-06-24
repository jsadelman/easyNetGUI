#include "historytreemodel.h"
#include "treeitem.h"
#include "enumclasses.h"
#include "sessionmanager.h"
#include "xmlelement.h"

#include "objectcachefilter.h"
#include "objectupdater.h"


#include <QDomDocument>


HistoryTreeModel::HistoryTreeModel(QObject *parent)
    : TreeModel(QStringList({""}), parent)
{
    filter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    updater = new ObjectUpdater(this);
    updater->setProxyModel(filter);
    connect(updater, SIGNAL(objectUpdated(QDomDocument*,QString)), this, SLOT(updatePrettyName(QDomDocument*,QString)));
}

QVariant HistoryTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() != 0)
        return QVariant();

    CheckRecord record = getItem(index)->data(0).value<CheckRecord>();

    if (!index.parent().isValid())
    {
        if (role == Qt::DisplayRole || NameRole)
            return record.name;
        else
            return QVariant();
    }
    else
    {
        switch (role)
        {
        case Qt::DisplayRole:
            return QString("%1 [%2]").arg(record.prettyName).arg(record.name);
        case Qt::CheckStateRole:
            return record.checked ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked);
        case NameRole:
            return record.name;
        case PrettyNameRole:
            return record.prettyName;
        default:
            return QVariant();
        }
    }
}

Qt::ItemFlags HistoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    else if (!index.parent().isValid())
        return Qt::ItemIsEnabled;

    else
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren;
}

int HistoryTreeModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

bool HistoryTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    TreeItem *item = getItem(index);
    QVariant v = item->data(0);
    CheckRecord record;
    if (v.canConvert<CheckRecord>())
        record = v.value<CheckRecord>();
    bool success = false;
    bool changed = false;

    switch(role)
    {
//    case Qt::EditRole:
//        record.name = value.toString();
//        v.setValue(record);
//        success = item->setData(0, v);
//        changed = true;
//        break;
    case NameRole:
    {
        QString name = value.toString();
        if (name != record.name)
        {
            record.name = name;
            v.setValue(record);
            success = item->setData(0, v);
            changed = true;
        }
        QDomDocument *description = SessionManager::instance()->description(name);
        QString prettyName;
        if (description)
            prettyName = XMLelement(*description)["pretty name"]();
        else
            prettyName = record.name;

        if (prettyName != record.prettyName)
        {
            record.prettyName = prettyName;
            v.setValue(record);
            success |= item->setData(0, v);
            changed = true;
        }
        break;
    }
    case PrettyNameRole:
    {
        QString prettyName = value.toString();
        if (prettyName != record.prettyName)
        {
            record.prettyName = prettyName;
            v.setValue(record);
            success = item->setData(0, v);
            changed = true;
        }
        break;
    }
    case Qt::CheckStateRole:
        if (index.parent().isValid())
        {
            if (record.checked != (value.toInt() == Qt::Checked))
            {
                record.checked = value.toInt() == Qt::Checked;
                v.setValue(record);
                success = item->setData(0, v);
                changed = true;
            }
            else
            {
                success = true;
            }
        }
    default:
        ;
    }
    if (success && changed)
    {
        if (role == Qt::CheckStateRole)
            emit dataChanged(index, index, QVector<int>({role}));
        else if (role == NameRole || role == PrettyNameRole || role == Qt::EditRole)
            emit dataChanged(index, index, QVector<int>({NameRole, PrettyNameRole, Qt::EditRole}));
    }

    return success;
}

bool HistoryTreeModel::containsTrial(QString trial)
{
    return trialIndex(trial).isValid();
}

bool HistoryTreeModel::containsView(QString view, QString trial)
{
    return viewIndex(view, trial).isValid();
}

bool HistoryTreeModel::containsView(QString view)
{
    return viewIndex(view).isValid();
}

bool HistoryTreeModel::isInView(QString view)
{
    return data(viewIndex(view), Qt::CheckStateRole).toInt() == Qt::Checked;
}

bool HistoryTreeModel::appendTrial(QString trial)
{
    if (!insertRows(rowCount(), 1))
    {
        eNerror << "cannot insert a new row for trial" << trial;
        return false;
    }
    if (!setData(index(rowCount()-1,0), trial, NameRole))
    {
        eNerror << "cannot set trial" << trial;
        return false;
    }
    return true;
}

bool HistoryTreeModel::removeTrial(QString trial)
{
    QModelIndex index = trialIndex(trial);
    if (!index.isValid())
    {
        eNwarning << QString("attempt to remove trial %1, which is not contained in this model").arg(trial);
        return false;
    }
    return removeRows(index.row(), 1);
}

bool HistoryTreeModel::appendView(QString view, QString trial, bool inView)
{
    if (!containsTrial(trial))
        appendTrial(trial);

    QModelIndex trialIdx = trialIndex(trial);

    if (!insertRows(rowCount(trialIdx), 1, trialIdx))
    {
        eNerror << "cannot insert a new row for view" << view;
        return false;
    }

    trialIdx = trialIndex(trial);
    if (!setData(index(rowCount(trialIdx)-1, 0, trialIdx), view, NameRole))
    {
        eNerror << "cannot set view" << view;
        return false;
    }
//    QDomDocument *description = SessionManager::instance()->description(view);
//    if (description && XMLelement(*description)["hints"]["show"]() == "1")
//        inView = true;
    if (!setData(index(rowCount(trialIdx)-1, 0, trialIdx), inView ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked), Qt::CheckStateRole))
    {
        eNerror << "cannot set check value for view" << view;
        return false;
    }
    return true;
}

bool HistoryTreeModel::removeView(QString view, QString tr)
{
    if (tr.isEmpty())
        tr = trial(view);

    QModelIndex index = viewIndex(view, tr);

    if (!index.isValid())
    {
        eNwarning << QString("attempt to remove view %1 from trial %2, which is not contained in this model").arg(view).arg(tr);
        return false;
    }
    QModelIndex trialIdx = trialIndex(tr);
    bool success = removeRows(index.row(), 1, trialIdx);
    if (rowCount(trialIdx) == 0)
        success &= removeRows(trialIdx.row(), 1);
    return success;
}

bool HistoryTreeModel::setInView(QString view, QString trial, bool inView)
{
    return setData(viewIndex(view, trial), inView ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked), Qt::CheckStateRole);
}

bool HistoryTreeModel::setInView(QString view, bool inView)
{
    setInView(view, trial(view), inView);
}


QModelIndex HistoryTreeModel::trialIndex(QString trial)
{
    QModelIndexList matchList = match(index(0,0), NameRole, trial, 1, Qt::MatchExactly);
    return matchList.isEmpty() ? QModelIndex() : matchList.first();
}

QModelIndex HistoryTreeModel::viewIndex(QString view, QString trial)
{
    QModelIndex trialIdx = trialIndex(trial);
    if (!trialIdx.isValid())
        return QModelIndex();

    QModelIndexList matchList = match(index(0, 0, trialIdx), NameRole, view, 1, Qt::MatchFixedString | Qt::MatchCaseSensitive);
    return matchList.isEmpty() ? QModelIndex() : matchList.first();
}

QModelIndex HistoryTreeModel::viewIndex(QString view)
{
    QModelIndex viewIdx = QModelIndex();
    for(int trialRow = 0; trialRow < rowCount(); ++trialRow)
    {
        if ((viewIdx = viewIndex(view, data(index(trialRow, 0), NameRole).toString())).isValid())
            return viewIdx;
    }
    return viewIdx;
}

QString HistoryTreeModel::trial(QString view)
{
    QModelIndex viewIdx = viewIndex(view);
    if (viewIdx.isValid())
        return data(viewIdx.parent(),  NameRole).toString();
    return QString();
}

void HistoryTreeModel::updatePrettyName(QDomDocument *description, QString name)
{
    if (description)
        setData(viewIndex(name), XMLelement(*description)["pretty name"](), PrettyNameRole);
}

