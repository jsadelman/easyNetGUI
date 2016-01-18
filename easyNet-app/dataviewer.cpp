#include "dataviewer.h"
#include "ui_dataviewer.h"
#include "sessionmanager.h"
#include "enumclasses.h"
#include "dataviewerdispatcher.h"
#include "objectcachefilter.h"

#include <QAction>
#include <QDomDocument>
#include <QVBoxLayout>

Q_DECLARE_METATYPE(QDomDocument*)

DataViewer::DataViewer(Ui_DataViewer *ui, QWidget *parent)
    : QWidget(parent), ui(ui), dispatcher(nullptr), m_lazy(false), lastOpenDir(""), lastSaveDir("")
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

void DataViewer::preDispatch(QDomDocument *info)
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
        if (!infoVariant.canConvert<QDomDocument *>())
        {
            eNerror << "unable to get trial run information";
            return;
        }
        QDomDocument* info = infoVariant.value<QDomDocument*>();
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
        enableActions(true);
}

void DataViewer::enableActions(bool enable)
{
    if (!ui)
        return;
    ui->saveAct->setEnabled(enable);
    ui->copyAct->setEnabled(enable);
    if (dispatcher)
        ui->setDispatchModeAutoAct->setEnabled(enable);
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

