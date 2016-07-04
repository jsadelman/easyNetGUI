#include "settingswidget.h"
#include "plotsettingsform.h"
#include "enumclasses.h"
#include "lazynutjob.h"
#include "sessionmanager.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "xmlelement.h"
#include "easyNetMainWindow.h"

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


SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent),
      currentName("")
{
//    createActions();
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

void SettingsWidget::sendSettings(QString name, bool force)
{
    name = name.isEmpty() ? currentName : name;
    PlotSettingsForm *form = formMap.value(name, nullptr);
    if (form)
    {
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = form->getSettingsCmdList(true); // force
        QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
                << job;
        if (force)
            jobs << SessionManager::instance()->updateObjectCacheJobs();
        else
            jobs << SessionManager::instance()->recentlyModifiedJob();

        if (force)
        {
            QMap<QString, QVariant> jobData;
            jobData.insert("name", name);
            jobs.last()->data = jobData;
            jobs.last()->appendEndOfJobReceiver(this, SLOT(emitDataViewCreated()));
        }
        SessionManager::instance()->submitJobs(jobs);
    }
}

void SettingsWidget::newForm(QString name, QString dataViewType, QString rScript, QMap<QString, QString> defaultSettings, bool isBackup)
{
    if (!(dataViewType == "dataframe_view" || dataViewType == "rplot"))
    {
        eNerror << "invalid dataViewType:" << dataViewType;
        return;
    }
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({
                                QString("create %1 %2").arg(dataViewType).arg(name),
                                QString("%1 set_type %2").arg(name).arg(rScript),
                                QString("xml %1 list_settings").arg(name)
                                });
    QMap<QString, QVariant> jobData;
    jobData.insert("name", name);
    if (!defaultSettings.isEmpty())
        jobData.insert("defaultSettings", QVariant::fromValue(defaultSettings));
    jobData.insert("isBackup", isBackup);
    job->setAnswerReceiver(this, SLOT(setCurrentSettings(QDomDocument*)), AnswerFormatterType::XML);
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job;
//            << SessionManager::instance()->updateObjectCacheJobs();
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
    else if (!visibleRegion().isEmpty())
        createNewForm(name);
}

void SettingsWidget::createNewForm(QString name)
{
    if (name.isEmpty())
    {
        LazyNutJob *job = qobject_cast<LazyNutJob *>(sender());
        if (!job)
        {
            eNerror << "cannot extract LazyNutJob from sender";
            return;
        }
        name = SessionManager::instance()->getDataFromJob(job, "name").toString();
        if (name.isEmpty())
        {
            eNerror << "LazyNutJob->data contains an empty name entry";
            return;
        }
    }
    if (!SessionManager::instance()->exists(name))
    {
//        eNerror << "object" << name << "does not exist";
        return;
    }
    QString subtype = SessionManager::instance()->descriptionCache->subtype(name);
    if (!(subtype == "dataframe_view" || subtype == "rplot"))
    {
//        eNerror << "object" << name << "is not of subtype rplot or dataframe_view";
        return;
    }
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("xml %1 list_settings").arg(name)});
    QMap<QString, QVariant> jobData;
    jobData.insert("name", name);
    job->setAnswerReceiver(this, SLOT(setCurrentSettings(QDomDocument*)), AnswerFormatterType::XML);
    job->appendEndOfJobReceiver(this, SLOT(buildSettingsForm()));
    job->data = jobData;
    SessionManager::instance()->submitJobs(job);
}

void SettingsWidget::buildSettingsForm(QString name, QDomDocument *domDoc, QMap<QString, QString> defaultSettings)
{
    PlotSettingsForm *settingsForm = new PlotSettingsForm(domDoc, this);
    settingsForm->setName(name);
    settingsForm->setDefaultSettings(defaultSettings);
    settingsForm->build();
    formMap[name] = settingsForm;
    setForm(name);
    sendSettings(name, true);
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
    bool isBackup(false);
    if (jobData.contains("isBackup"))
        isBackup = jobData.value("isBackup").toBool();

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
    descriptionFilter->addName(name);
//    emit dataViewCreated(name, isBackup);
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

void SettingsWidget::apply()
{
    if (!currentName.isEmpty())
    {
        sendSettings();
        emit settingsApplied(currentName);
    }
}


void SettingsWidget::reloadScript()
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
    descriptionFilter->removeName(name);
}

void SettingsWidget::emitDataViewCreated()
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
    emit dataViewCreated(name, false);
}

void SettingsWidget::buildWidget()
{
    reloadScriptButton = new QPushButton("Reload script",this);
    connect(reloadScriptButton, SIGNAL(clicked()), this, SLOT(reloadScript()));
    connect(MainWindow::instance(), SIGNAL(debugModeChanged(bool)), reloadScriptButton, SLOT(setVisible(bool)));

    applyButton = new QPushButton("Apply", this);
    connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));

    okButton = new QPushButton("Ok", this);
    connect(okButton, &QPushButton::clicked, [=]()
    {
       apply();
       emit closeRequested();
    });

    QVBoxLayout *buttonsLayout = new QVBoxLayout;
    buttonsLayout->addWidget(applyButton);
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(reloadScriptButton);

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

