#include "settingswidget.h"
#include "plotsettingsform.h"
#include "enumclasses.h"
#include "lazynutjob.h"
#include "sessionmanager.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "xmlelement.h"

#include <QAction>
#include <QToolButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QScrollArea>
#include <QDomDocument>
#include <QFileInfo>

Q_DECLARE_METATYPE(QSharedPointer<QDomDocument> )


SettingsWidget::SettingsWidget(QString dataViewType, QWidget *parent)
    : QWidget(parent),
      dataViewType(dataViewType),
      currentName("")
{
    if (!(dataViewType == "dataframe_view" || dataViewType == "rplot"))
    {
        eNerror << "invalid dataViewType:" << dataViewType;
        return;
    }
    createActions();
    buildWidget();
    descriptionFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    descriptionUpdater = new ObjectUpdater(this);
    descriptionUpdater->setProxyModel(descriptionFilter);
    connect(descriptionUpdater, &ObjectUpdater::objectUpdated, [=](QDomDocument* domDoc, QString name)
    {
        typeMap[name] = QFileInfo(XMLelement(*domDoc)["Type"]()).fileName();
        if (name == currentName)
            typeEdit->setText(typeMap[name]);
    });
}

SettingsWidget::~SettingsWidget()
{

}

void SettingsWidget::setSetting(QString setting, QString value)
{
    PlotSettingsForm *form = qobject_cast<PlotSettingsForm *>(formScrollArea->widget());
    if (form)
        form->setSetting(setting, value);
}

QMap<QString, QString> SettingsWidget::getSettings(QString name)
{
    if (!formMap.contains(name))
    {
        eNerror << QString("name %1 does not match any settings form").arg(name);
        return QMap<QString, QString>();
    }
    return formMap.value(name)->getSettings();
}

void SettingsWidget::sendSettings(QString name)
{
    PlotSettingsForm *form = formMap.value(name.isEmpty() ? currentName : name, nullptr);
    if (form)
    {
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = form->getSettingsCmdList();
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job
                << SessionManager::instance()->recentlyModifiedJob();
        SessionManager::instance()->submitJobs(jobs);
    }
}

void SettingsWidget::newForm(QString name, QString rScript, QMap<QString, QString> defaultSettings, int flags, QList<QSharedPointer<QDomDocument> > infoList)
{
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER; // debug purpose
    job->cmdList = QStringList({
                                QString("create %1 %2").arg(dataViewType).arg(name),
                                QString("%1 set_type %2").arg(name).arg(rScript),
                                QString("xml %1 list_settings").arg(name)
                                });
    QMap<QString, QVariant> jobData;
    jobData.insert("name", name);
    if (!defaultSettings.isEmpty())
        jobData.insert("defaultSettings", QVariant::fromValue(defaultSettings));
    jobData.insert("flags", flags);
    QList<QVariant> vList;
    foreach(QSharedPointer<QDomDocument> info, infoList)
        vList.append(QVariant::fromValue(info));
    jobData.insert("trialRunInfo", vList);
    job->setAnswerReceiver(this, SLOT(setCurrentSettings(QDomDocument*)), AnswerFormatterType::XML);

    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->recentlyCreatedJob();
    jobs.last()->appendEndOfJobReceiver(this, SLOT(buildSettingsForm()));
    jobs.last()->data = jobData;
    SessionManager::instance()->submitJobs(jobs);
}

void SettingsWidget::clearForm()
{
    formScrollArea->takeWidget();
    nameEdit->clear();
    typeEdit->clear();
    currentName.clear();
}

QString SettingsWidget::type(QString name)
{
    QDomDocument *domDoc = descriptionFilter->getDomDoc(name);
    if (domDoc)
        return QFileInfo(XMLelement(*domDoc)["Type"]()).fileName();
    return QString();
}

void SettingsWidget::setForm(QString name)
{
    clearForm();
    if (formMap.contains(name))
    {
        currentName = name;
        nameEdit->setText(name);
        typeEdit->setText(typeMap.value(name));
        formScrollArea->setWidget(formMap.value(name));
    }
}

void SettingsWidget::buildSettingsForm(QString name, QDomDocument *domDoc, QMap<QString, QString> defaultSettings)
{
    PlotSettingsForm *settingsForm = new PlotSettingsForm(domDoc, this);
    settingsForm->setName(name);
    settingsForm->setDefaultSettings(defaultSettings);
    settingsForm->build();
    formMap[name] = settingsForm;
    setForm(name);
}

void SettingsWidget::buildSettingsForm()
{
    LazyNutJob *job = qobject_cast<LazyNutJob *>(sender());
    if (!job)
    {
        eNerror << "cannot extract LazyNutJob from sender";
        return;
    }
    QMap<QString, QVariant> jobData = job->data.toMap();
    if (!jobData.contains("name"))
    {
        eNerror << "LazyNutJob->data does not contain name entry";
        return;
    }
    QString name = jobData.value("name").toString();
    if (name.isEmpty())
    {
        eNerror << "LazyNutJob->data contains an empty name entry";
        return;
    }
    QMap<QString, QString> defaultSettings;
    if (jobData.contains("defaultSettings"))
        defaultSettings = jobData.value("defaultSettings").value<QMap<QString, QString>>();
    int flags = 0;
    if (jobData.contains("flags"))
        flags = jobData.value("flags").toInt();
//    SessionManager::instance()->setPlotFlags(name, flags);

    QList<QSharedPointer<QDomDocument> > info;
    QVariant v = SessionManager::instance()->getDataFromJob(sender(), "trialRunInfo");
    if (v.canConvert<QList<QVariant> >())
    {
        foreach(QVariant vi, v.toList())
        {
            if (vi.canConvert<QSharedPointer<QDomDocument> >())
                info.append(vi.value<QSharedPointer<QDomDocument> >());
        }
    }
    // add values != NULL or "" to defaultSettings
    QMap<QString, QString> completeDefaultSettings;
    QDomElement domElement = currentSettings->documentElement().firstChildElement();
    while (!domElement.isNull())
    {
        QDomElement valueElement = XMLAccessor::childElement(domElement, "value");
        QString value = XMLAccessor::value(valueElement);
        if (!value.isEmpty() && value != "NULL")
            completeDefaultSettings[XMLAccessor::label(domElement)] = value;
        domElement = domElement.nextSiblingElement();
    }
    foreach(QString setting, defaultSettings.keys())
        completeDefaultSettings[setting] = defaultSettings[setting];


    buildSettingsForm(name, currentSettings, completeDefaultSettings);
    descriptionFilter->setName(name);
    emit dataViewCreated(name, true, false, info);
}

void SettingsWidget::rebuildForm()
{
    delete formMap.value(currentName, nullptr);
    // add values != NULL or "" to defaultSettings
    QMap<QString, QString> completeDefaultSettings;
    QDomElement domElement = currentSettings->documentElement().firstChildElement();
    while (!domElement.isNull())
    {
        QDomElement valueElement = XMLAccessor::childElement(domElement, "value");
        QString value = XMLAccessor::value(valueElement);
        if (!value.isEmpty() && value != "NULL")
            completeDefaultSettings[XMLAccessor::label(domElement)] = value;
        domElement = domElement.nextSiblingElement();
    }
    buildSettingsForm(currentName, currentSettings, completeDefaultSettings);
    setForm(currentName);
}


void SettingsWidget::refreshForm()
{
    if (currentName.isEmpty())
        return;
    sendSettings();
    LazyNutJob *job = new LazyNutJob;
    job->cmdList   << QString("%1 set_type %2").arg(currentName).arg(type(currentName))
                   << QString("xml %1 list_settings").arg(currentName);
    job->setAnswerReceiver(this, SLOT(setCurrentSettings(QDomDocument*)), AnswerFormatterType::XML);
    job->appendEndOfJobReceiver(this, SLOT(rebuildForm()));
    SessionManager::instance()->submitJobs(job);
}

void SettingsWidget::removeForm(QString name)
{
    if (name == currentName)
        clearForm();
    delete formMap.value(name, nullptr);
    formMap.remove(name);
    typeMap.remove(name);
}

void SettingsWidget::createActions()
{
    refreshAct = new QAction(this);
    refreshAct->setShortcuts(QKeySequence::Refresh);
    refreshAct->setToolTip("reload current settings form from R script");
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refreshForm()));

    applyAct = new QAction(this);
    applyAct->setToolTip("apply current settings");
    connect(applyAct, &QAction::triggered, this, [=]{
        if (!currentName.isEmpty())
        {
            sendSettings();
        }
    });
}

void SettingsWidget::buildWidget()
{
    refreshButton = new QToolButton(this);
    refreshButton->setAutoRaise(true);
    refreshButton->setDefaultAction(refreshAct);
    refreshButton->setIcon(QIcon(":/images/refresh.png"));
    refreshButton->setIconSize(QSize(40, 40));

    applyButton = new QToolButton(this);
    applyButton->setAutoRaise(true);
    applyButton->setDefaultAction(applyAct);
    applyButton->setIcon(QIcon(":/images/media-play-8x.png"));
    applyButton->setIconSize(QSize(40, 40));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(applyButton);
    buttonsLayout->addWidget(refreshButton);

    nameEdit = new QLineEdit;
    nameEdit->setReadOnly(true);
    typeEdit = new QLineEdit;
    typeEdit->setReadOnly(true);
    QFormLayout *nameTypeLayout = new QFormLayout;
    nameTypeLayout->addRow("Name", nameEdit);
    nameTypeLayout->addRow("Type", typeEdit);

    formScrollArea = new QScrollArea;
    formScrollArea->setWidgetResizable(true);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addLayout(nameTypeLayout);
    topLayout->addLayout(buttonsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(formScrollArea);
    setLayout(mainLayout);
}

