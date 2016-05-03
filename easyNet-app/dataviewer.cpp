#include "dataviewer.h"
#include "ui_dataviewer.h"
#include "sessionmanager.h"
#include "enumclasses.h"
#include "dataviewerdispatcher.h"
#include "objectcachefilter.h"
#include "objectupdater.h"


#include <QAction>
#include <QDomDocument>
#include <QVBoxLayout>
#include <QSharedPointer>

Q_DECLARE_METATYPE(QSharedPointer<QDomDocument> )

DataViewer::DataViewer(Ui_DataViewer *ui, QWidget *parent)
    : QWidget(parent), ui(ui), dispatcher(nullptr), m_lazy(false),
      lastOpenDir(""), defaultOpenDir(""), lastSaveDir(""), defaultSaveDir("")
{
    setUi();
    descriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
//    descriptionFilter->setAllPassFilter(); // may be specialised by derived classes
    connect(descriptionFilter, SIGNAL(objectDestroyed(QString)), this, SLOT(destroyItem(QString)));
    descriptionUpdater = new ObjectUpdater(this);
    descriptionUpdater->setCommand("description");
    descriptionUpdater->setProxyModel(descriptionFilter);
    connect(descriptionUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)), this, SLOT(execAddItem(QDomDocument*,QString)));
    m_items.clear();
}

DataViewer::~DataViewer()
{
    //delete ui;
    //delete dispatcher;
}

void DataViewer::setUi()
{
    if (!ui)
    {
        eNerror << "invalid Ui_DataViewer object";
        return;
    }
//    qDebug()<<(ui->setup?"Y":"N");
//    if(!ui->setup)
    {
      ui->setupUi(this);
      QVBoxLayout *mlayout = new QVBoxLayout;
      mlayout->addWidget(ui);
      setLayout(mlayout);
     }

    enableActions(false);
}

void DataViewer::execAddItem(QDomDocument *domDoc, QString name)
{

    Q_UNUSED(domDoc)
    if (name.isEmpty())
    {
        eNerror << "name is empty";
        return;
    }

    if (contains(name))
    {
        if (dispatcher)
        {
            dispatcher->updateInfo(name);
            dispatcher->updateHistory(name);
        }
        return;
    }
    else
    {
        addItem_impl(name);
        if (dispatcher)
        {
            dispatcher->addToHistory(name, !isBackupMap.value(name, false));
            isBackupMap.remove(name);
        }
        else
        {
            addView(name);
//            ui->setCurrentItem(name);
        }
        m_items.append(name);
        if(m_items.size()==1)ui->setCurrentItem(name);
    }
}

void DataViewer::initiateDestroyItem(QString name)
{

    if (dispatcher && sender() == ui)
    {
        dispatcher->setInView(name, false);
    }
    else if (name.contains(QRegExp("[()]"))) // don't destroy default dataframes
    {
        eNwarning << QString("attempt to delete lazyNut system object %1").arg(name);
        // this should change, trigger desable default observer
    }
    else
    {
        emit itemRemoved(name);
        if (SessionManager::instance()->exists(name))
        {
            SessionManager::instance()->destroyObject(name);
        }
        else
        {
            SessionManager::instance()->removeFromExtraNamedItems(name);
            destroyItem(name);
        }
    }
}

void DataViewer::destroyItem(QString name)
{
    if (!contains(name))
    {
        eNwarning << QString("attempt to delete non-existing item %1").arg(name) << sender()->metaObject()->className();
        return;
    }
    if (dispatcher)
        dispatcher->removeFromHistory(name);
    destroyItem_impl(name);
    removeView(name);
    m_items.removeAll(name);
}


void DataViewer::setDispatcher(DataViewerDispatcher *dataViewerDispatcher)
{
    dispatcher = dataViewerDispatcher;
    if (!dispatcher)
    {
        eNerror << "invalid DataViewerDispatcher object";
        return;
    }
    if (ui)
    {
//        ui->setDispatchModeAutoAct->setVisible(true);
//        setDispatchModeAuto(true);
//        connect(descriptionUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)), dispatcher, SLOT(updateInfo(QDomDocument*,QString)));
//        connect(descriptionUpdater, &ObjectUpdater::objectUpdated, [=](QDomDocument*,QString name)
//        {
//            dispatcher->updateHistory(name);
//        });
//        ui->setDispatchModeAutoAct->setEnabled(enable);
//        dispatcher->historyAct->setEnabled(enable);
    }
}

bool DataViewer::contains(QString name)
{
    if (dispatcher)
        return dispatcher->inHistory(name);
    if (ui)
        return ui->contains(name);
    return false;
}

void DataViewer::setDefaultDir(QString dir)
{
    setDefaultOpenDir(dir);
    setDefaultSaveDir(dir);
}

void DataViewer::addView(QString name)
{
    ui->addView(name, makeView(name));
    if (!isLazy())
        addNameToFilter(name);
}

void DataViewer::removeView(QString name)
{
    if (!isLazy())
        removeNameFromFilter(name);
    delete ui->takeView(name);
    //    descriptionFilter->removeName(name);
}

QString DataViewer::currentItemName()
{
    return ui->currentItemName();
}

void DataViewer::addItem(QString name, bool isBackup)
{
    if (name.isEmpty())
    {
        QVariant v = SessionManager::instance()->getDataFromJob(sender(), "name");
        if (!v.canConvert<QString>())
        {
            eNerror << "cannot retrieve a valid string from name key in sender LazyNut job";
            return;
        }
        name = v.value<QString>();
        v = SessionManager::instance()->getDataFromJob(sender(), "isBackup");
        if (v.canConvert<bool>())
            isBackup = v.value<bool>();
    }
    if (name.isEmpty())
    {
        eNerror << "name is empty";
        return;
    }

    if (SessionManager::instance()->exists(name))
    {
        isBackupMap[name] = isBackup;
        descriptionFilter->addName(name);
    }
    else if (SessionManager::instance()->extraNamedItems().contains(name))
    {
        isBackupMap[name] = isBackup;
        execAddItem(nullptr, name);
    }
    else
    {
        eNerror << QString("name %1 is not recognised as valid").arg(name);
        return;
    }
}


void DataViewer::preDispatch(QSharedPointer<QDomDocument> info)
{
    if (!info)
        eNerror << "not a valid trial run info pointer";
    else if (!dispatcher)
        eNerror << "received a trial run info object but no dispatcher set";
    else
        dispatcher->preDispatch(info);
}

void DataViewer::dispatch()
{
//    if (!info)
//    {
        QVariant infoVariant = SessionManager::instance()->getDataFromJob(sender(), "trialRunInfo");
        if (!infoVariant.canConvert<QSharedPointer<QDomDocument> >())
        {
            eNerror << "unable to get trial run information";
            return;
        }
        QSharedPointer<QDomDocument> info = infoVariant.value<QSharedPointer<QDomDocument> >();
//    }
    if (!dispatcher)
        eNerror << "received a trial run info object but no dispatcher set";
    else
        dispatcher->dispatch(info);
}

void DataViewer::setCurrentItem(QString name)
{
    qDebug() << name <<" " << "!";
    if (name.isEmpty() || name == "<select an item>"  || ! m_items.contains(name))
        enableActions(false);
    else
    {
        qDebug() <<" gotcha";
        enableActions(true);
        if (isLazy())
            setNameInFilter(name);
        ui->setToolBars(this);
    }
    emit currentItemChanged(name);
}

void DataViewer::enableActions(bool enable)
{
    if (!ui)
        return;
    ui->saveAct->setEnabled(enable);
    ui->copyAct->setEnabled(enable);
    ui->destroyAct->setEnabled(enable);
    ui->settingsAct->setEnabled(enable);

}

//void DataViewer::setTrialRunInfo(QString item, QSharedPointer<QDomDocument> info)
//{
//    if (dispatcher)
//    {
//        dispatcher->setTrialRunInfo(item, info);
//    }
//    else
//    {
//        eNerror << "no dispatcher set for this viewer";
//    }
//}

void DataViewer::setTrialRunMode(int mode)
{
    if (dispatcher)
        dispatcher->setTrialRunMode(mode);
}

//void DataViewer::setDispatchModeOverride(int mode)
//{
//    if (dispatcher)
//    {
//        dispatcher->dispatchModeOverride = mode;
//    }
//}

//void DataViewer::setDispatchModeAuto(bool isAuto)
//{
//    if (dispatcher)
//    {
//        dispatcher->dispatchModeAuto = isAuto;
//        ui->setDispatchModeOverrideActGroup->setVisible(!isAuto);
//        if (isAuto)
//            dispatcher->dispatchModeOverride = -1;
//        else
//            dispatcher->restoreOverrideDefaultValue();
//    }
//}

void DataViewer::destroySelectedItems()
{
    if (dispatcher && dispatcher->historyAct->isChecked())
        dispatcher->destroySelectedItems();
    else
        initiateDestroyItem(ui->currentItemName());
}

