#include "dataviewer.h"
#include "ui_dataviewer.h"
#include "sessionmanager.h"
#include "enumclasses.h"
#include "dataviewerdispatcher.h"
#include "objectcachefilter.h"

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
    destroyedObjectsFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    destroyedObjectsFilter->setAllPassFilter(); // may be specialised by derived classes
    connect(destroyedObjectsFilter, SIGNAL(objectDestroyed(QString)), this, SLOT(removeItem(QString)));
}

DataViewer::~DataViewer()
{
    delete ui;
    delete dispatcher;
}

void DataViewer::setUi()
{
    if (!ui)
    {
        eNerror << "invalid Ui_DataViewer object";
        return;
    }
    ui->setupUi(this);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(ui);
    setLayout(layout);
    enableActions(false);
    connect(ui, SIGNAL(deleteItemRequested(QString)), this, SLOT(initiateRemoveItem(QString)));
    connect(ui, SIGNAL(currentItemChanged(QString)), this, SLOT(updateCurrentItem(QString)));

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
        ui->setDispatchModeAutoAct->setVisible(true);
        setDispatchModeAuto(true);
    }
}

bool DataViewer::contains(QString name)
{
    if (ui)
        return ui->contains(name);
    return false;
}

void DataViewer::setDefaultDir(QString dir)
{
    setDefaultOpenDir(dir);
    setDefaultSaveDir(dir);
}

void DataViewer::addItem(QString name, bool setCurrent, bool isBackup)
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
        v = SessionManager::instance()->getDataFromJob(sender(), "setCurrent");
        if (v.canConvert<bool>())
            setCurrent = v.value<bool>();
        v = SessionManager::instance()->getDataFromJob(sender(), "isBackup");
        if (v.canConvert<bool>())
            isBackup = v.value<bool>();
    }
    if (name.isEmpty())
    {
        eNerror << "name is empty";
    }
    else if (!SessionManager::instance()->exists(name))
    {
        eNerror << QString("attempt to add a non-existing object %1").arg(name);
    }
    else if (ui->contains(name))
    {
        if (setCurrent)
            ui->setCurrentItem(name);
    }
    else
    {
        addItem_impl(name);
        if (dispatcher && isBackup)
        {
            dispatcher->addToHistory(name);
        }
        else
        {
            ui->addView(name, makeView());
            if (!isLazy())
                addNameToFilter(name);
            if (setCurrent)
                ui->setCurrentItem(name);
        }
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

void DataViewer::updateCurrentItem(QString name)
{
    if (name.isEmpty() || name == "<select an item>")
        enableActions(false);
    else
    {
        enableActions(true);
        if (isLazy())
            setNameInFilter(name);
    }
}

void DataViewer::enableActions(bool enable)
{
    if (!ui)
        return;
    ui->saveAct->setEnabled(enable);
    ui->copyAct->setEnabled(enable);
    if (dispatcher)
    {
        ui->setDispatchModeAutoAct->setEnabled(enable);
        dispatcher->historyAct->setEnabled(enable);
    }
}

void DataViewer::setTrialRunInfo(QString item, QSharedPointer<QDomDocument> info)
{
    if (dispatcher)
    {
        dispatcher->setTrialRunInfo(item, info);
    }
    else
    {
        eNerror << "no dispatcher set for this viewer";
    }
}

void DataViewer::setDispatchModeOverride(int mode)
{
    if (dispatcher)
    {
        dispatcher->dispatchModeOverride = mode;
    }
}

void DataViewer::setDispatchModeAuto(bool isAuto)
{
    if (dispatcher)
    {
        dispatcher->dispatchModeAuto = isAuto;
        ui->setDispatchModeOverrideActGroup->setVisible(!isAuto);
    }
}

