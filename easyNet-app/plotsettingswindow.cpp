
#include <QtWidgets>
#include <QDomDocument>
#include <QListView>
#include <QScrollArea>
#include <QMapIterator>
#include <QInputDialog>

#include "plotsettingswindow.h"
#include "codeeditor.h"
#include "lazynutjob.h"
#include "sessionmanager.h"
#include "plotsettingsform.h"
#include "xmlelement.h"
#include "plotviewer_old.h"
#include "objectcachefilter.h"
#include "xmlaccessor.h"


Q_DECLARE_METATYPE(QSharedPointer<QDomDocument> )


PlotSettingsWindow::PlotSettingsWindow(QWidget *parent)
    : createNewPlotText("Create new plot"),
      openPlotSettingsText("Open plot settings"),
      savePlotSettingsText("Save plot settings"),
      savePlotSettingsAsText("Save plot setings as..."),
//      plotSettingsForm(nullptr),
      QMainWindow(parent),
      plotAspr_(1.),
      quietly(false)
{
    setUnifiedTitleAndToolBarOnMac(true);
    createActions();
    buildWindow();
}

int PlotSettingsWindow::getValueFromByteArray(QByteArray ba, QString key)
{
    QMessageBox msgBox;
    QByteArray w;
    int value;
    bool ok;
    int p = ba.indexOf(key, 0);
    if (p>-1)
    {
        w = ba.mid(p+key.size()+2, 3);
        value = w.toInt(&ok, 10);
        if (!ok)
        {
            QString printable = QString("Problem reading value. String was %1").arg(QString(w));
            msgBox.setText(printable);
             msgBox.exec();
        }
/*
 *         else
        {
            QString printable = QString("%1 is %2. String was %3").arg(key).arg(value).arg(QString(w));
            msgBox.setText(printable);
            msgBox.exec();
        }
*/
    }
    return(value);
}

void PlotSettingsWindow::openPlotSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open plot settings file"),
                                                    settings.value("plotSettingsDir").toString(), tr("Settings Files (*.xml)"));
    if (!fileName.isEmpty())
        loadSettings(fileName);
}

void PlotSettingsWindow::loadSettings(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    QDomDocument *domDoc = new QDomDocument;
    if (!domDoc->setContent(&file))
    {
        file.close();
        return;
    }
    file.close();
    buildSettingsForm(currentPlotName, domDoc);
    // ...
}

void PlotSettingsWindow::createActions()
{
    refreshAct = new QAction(this);
    refreshAct->setShortcuts(QKeySequence::Refresh);
    refreshAct->setToolTip("reload current plot settings form");
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refreshForm()));

    plotAct = new QAction(this);
    plotAct->setToolTip("plot according to current settings");
    connect(plotAct, &QAction::triggered, this, [=]{
        if (!currentPlotName.isEmpty())
        {
            sendSettings();
            emit showPlotViewer();
            sendGetCmd();
        }
    });
    newPlotAct = new QAction(QIcon(":/images/add-icon.png"), tr("&New Plot"), this);
    newPlotAct->setShortcuts(QKeySequence::New);
    connect(newPlotAct, SIGNAL(triggered()), this, SLOT(newPlot()));

//    enableActions(false);
}

void PlotSettingsWindow::triggerRefresh()
{
    sendSettings();
    emit showPlotViewer();
    sendGetCmd();
}

void PlotSettingsWindow::updateRecentRScriptsActs()
{
    QSettings settings("QtEasyNet", "nmConsole");
    QStringList rScripts = settings.value("recentRScripts","").toStringList();
    int numRecentRScripts = qMin(rScripts.size(), (int)MaxRecentRScripts);
    for (int i = 0; i < numRecentRScripts; ++i)
    {
        QString fileName = QFileInfo(rScripts.at(i)).fileName();
        QString text = fileName; //tr("&%1 %2").arg(i + 1).arg(fileName);
        recentRScriptsActs[i]->setText(text);
        recentRScriptsActs[i]->setData(fileName);
        recentRScriptsActs[i]->setVisible(true);
    }
    for (int i = numRecentRScripts; i < MaxRecentRScripts; ++i)
        recentRScriptsActs[i]->setVisible(false);

        separatorAct->setVisible(numRecentRScripts > 0);
}

void PlotSettingsWindow::importHomonyms(QDomDocument *settingsList)
{
//    XMLelement xml = XMLelement(*settingsList);
//    foreach (QString label, xml.listLabels())
//    {
//        if (plotSettingsForm->listLabels().contains(label))
//        {
//            QString value = plotSettingsForm->value(label);
//            if (value.isEmpty())
//                xml[label]["value"].setValue(xml[label]["default"]());

//            else
//                xml[label]["value"].setValue(value);
//        }
    //    }
}


void PlotSettingsWindow::sendGetCmd(QString plotName)
{
    if (!SessionManager::instance()->exists(plotName))
        return;
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("%1 get %2").arg(plotName).arg(plotAspr_)});
    job->setAnswerReceiver(this, SLOT(displaySVG(QByteArray, QString)), AnswerFormatterType::SVG);
    SessionManager::instance()->submitJobs(job);
}

void PlotSettingsWindow::sendGetCmd()
{
    if (plotControlPanelScrollArea->widget())
        sendGetCmd(plotForms.key(
                        qobject_cast<PlotSettingsForm*>(plotControlPanelScrollArea->widget())));
}

void PlotSettingsWindow::sendDrawCmd(QString plotName)
{
    sendSettings(plotName);
    sendGetCmd(plotName);
}

void PlotSettingsWindow::refreshForm()
{
    if (currentPlotName.isEmpty())
        return;
    sendSettings();
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("xml %1 list_settings").arg(currentPlotName)});
    job->setAnswerReceiver(this, SLOT(setCurrentSettings(QDomDocument*)), AnswerFormatterType::XML);
    job->appendEndOfJobReceiver(this, SLOT(rebuildForm()));
    SessionManager::instance()->submitJobs(job);
}

void PlotSettingsWindow::displaySVG(QByteArray plotByteArray, QString cmd)
{
    QString plotName = cmd.remove(QRegExp(" get .*$")).simplified();
//    replace tags that QSvgWidget doesn't like
    plotByteArray.replace (QByteArray("<symbol"),QByteArray("<g     "));
    plotByteArray.replace (QByteArray("</symbol"),QByteArray("</g     "));

//    get dimensions and set plot size accordingly
    int width = getValueFromByteArray(plotByteArray, "width");
    int height = getValueFromByteArray(plotByteArray, "height");
    if (width<1)
        width = 500;
    if (height<1)
        height = 500;
    QSize size(width,height); // plot_svg->sizeHint();
//    plot_svg->setMaximumSize(size);
//    plot_svg->setMinimumSize(size);

    emit plot(plotName, plotByteArray);
}

void PlotSettingsWindow::setPlotSettings(QString name)
{
    // in case name does not match an existing plot all will be set blank
    currentPlotName = name;
    plotControlPanelScrollArea->takeWidget();
    plotControlPanelScrollArea->setWidget(plotForms[currentPlotName]);
    plotNameBox->setText(name);
    plotTypeBox->setText(plotTypes[name]);
//    enableActions(plotControlPanelScrollArea->widget());
}


void PlotSettingsWindow::newPlot()
{
    NewPlotWizard *plotWizard = new NewPlotWizard(this);
    connect(plotWizard, SIGNAL(newRPlotCreated(QString,QString)),
            this, SLOT(newRPlot(QString,QString)));
//    connect(plotWizard, &NewPlotWizard::createNewRPlot, [=](QString name, QString type)
//    {
//       emit newRPlotCreated(name, type, QMap<QString,QString>(),QMap<QString,QString>(), false, -1);
//    });
    plotWizard->show();
}


void PlotSettingsWindow::newRPlot(QString name, QString rScript,
                                     QMap <QString,QString> defaultSettings, int flags, QList<QSharedPointer<QDomDocument> > infoList)
{
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER; // debug purpose
    job->cmdList = QStringList({
                                QString("create rplot %1").arg(name),
                                QString("%1 set_type %2").arg(name).arg(rScript),
                                QString("xml %1 list_settings").arg(name)
                                });
    QMap<QString, QVariant> jobData;
    jobData.insert("plotName", name);
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
    plotTypes[name] = rScript;
}


void PlotSettingsWindow::getSettingsXML(QString plotName)
{
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("xml %1 list_settings").arg(plotName)});
    job->setAnswerReceiver(this, SLOT(setCurrentSettings(QDomDocument*)), AnswerFormatterType::XML);
    QMap<QString, QVariant> data;
    data.insert("plotName", plotName);
    job->data = data;
    job->appendEndOfJobReceiver(this, SLOT(buildSettingsForm()));
    SessionManager::instance()->submitJobs(job);
}

void PlotSettingsWindow::buildSettingsForm()
{
    LazyNutJob *job = qobject_cast<LazyNutJob *>(sender());
    if (!job)
    {
        eNerror << "cannot extract LazyNutJob from sender";
        return;
    }
    QMap<QString, QVariant> jobData = job->data.toMap();
    if (!jobData.contains("plotName"))
    {
        eNerror << "LazyNutJob->data does not contain plotName entry";
        return;
    }
    QString plotName = jobData.value("plotName").toString();
    if (plotName.isEmpty())
    {
        eNerror << "LazyNutJob->data contains an empty plotName entry";
        return;
    }
    QMap<QString, QString> defaultSettings;
    if (jobData.contains("defaultSettings"))
        defaultSettings = jobData.value("defaultSettings").value<QMap<QString, QString>>();
    int flags = 0;
    if (jobData.contains("flags"))
        flags = jobData.value("flags").toInt();
    SessionManager::instance()->setPlotFlags(plotName, flags);

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


    buildSettingsForm(plotName, currentSettings, completeDefaultSettings);
    emit newRPlotCreated(plotName, !(flags & Plot_Backup), flags & Plot_Backup, info);
}

void PlotSettingsWindow::rebuildForm()
{
    delete plotForms.value(currentPlotName, nullptr);
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
    buildSettingsForm(currentPlotName, currentSettings, completeDefaultSettings);
}



void PlotSettingsWindow::buildSettingsForm(QString plotName, QDomDocument *domDoc, QMap<QString, QString> defaultSettings)
{
    PlotSettingsForm *plotSettingsForm = new PlotSettingsForm(domDoc, this);
    plotSettingsForm->setPlotName(plotName);
    plotSettingsForm->setDefaultSettings(defaultSettings);
    plotSettingsForm->build();
    plotForms[plotName] = plotSettingsForm;
}

void PlotSettingsWindow::buildWindow()
{
    QString backCol("white");
    QString textCol("black");
    QLabel* pnbLabel = new QLabel("Name:",this);
    QLabel* ptbLabel = new QLabel("Type:",this);
    pnbLabel->setStyleSheet("QLabel {"
                            "background-color: " + backCol + ";"
                            "color: " + textCol + ";"
                             "border: 1px solid black;"
                             "padding: 4px;"
                             "font: bold 12pt;"
                             "}");
    ptbLabel->setStyleSheet("QLabel {"
                            "background-color: " + backCol + ";"
                            "color: " + textCol + ";"
                             "border: 1px solid black;"
                             "padding: 4px;"
                             "font: bold 12pt;"
                             "}");

    plotNameBox = new QLabel(this);
    plotNameBox->setText(currentPlotName);
    plotNameBox->setStyleSheet("QLabel {"
                               "background-color: " + backCol + ";"
                             "color: " + textCol + ";"
                             "padding: 4px;"
                             "font: bold 12pt;"
                             "}");

    plotTypeBox = new QLabel(this);
    plotTypeBox->setText(currentPlotType);
    plotTypeBox->setStyleSheet("QLabel {"
                               "background-color: " + backCol + ";"
                             "color: " + textCol + ";"
                             "padding: 4px;"
                             "font: bold 12pt;"
                             "}");

    QToolButton*refreshButton = new QToolButton(this);
    refreshButton->setAutoRaise(true);
    refreshButton->setDefaultAction(refreshAct);
    refreshButton->setIcon(QIcon(":/images/refresh.png"));
    refreshButton->setIconSize(QSize(40, 40));
    refreshButton->show();

    QToolButton* newButton = new QToolButton(this);
    newButton->setAutoRaise(true);
    newButton->setDefaultAction(newPlotAct);
    newButton->setIcon(QIcon(":/images/add-icon.png"));
    newButton->setIconSize(QSize(40, 40));
    newButton->show();

    QToolButton* plotButton = new QToolButton(this);
    plotButton->setAutoRaise(true);
    plotButton->setDefaultAction(plotAct);
    plotButton->setIcon(QIcon(":/images/barchart2.png"));
    plotButton->setIconSize(QSize(40, 40));
    plotButton->show();

    gridLayout = new QGridLayout();
    gridLayout->addWidget(pnbLabel,0,0,1,1);
    gridLayout->addWidget(plotNameBox,0,1,1,6);
    gridLayout->addWidget(ptbLabel,1,0,1,1);
    gridLayout->addWidget(plotTypeBox,1,1,1,6);
    gridLayout->addWidget(plotButton,0,7,2,1);
    gridLayout->addWidget(refreshButton,0,8,2,1);
    gridLayout->addWidget(newButton,0,9,2,1);

    plotControlPanelScrollArea = new QScrollArea;
//    plotControlPanelScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    plotControlPanelScrollArea->setWidgetResizable(true);
    QWidget* dummyWidget = new QWidget(this);
    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addLayout(gridLayout);
    vlayout->addWidget(plotControlPanelScrollArea);
    dummyWidget->setLayout(vlayout);
    setCentralWidget(dummyWidget);
}

void PlotSettingsWindow::sendSettings(QString name)
{
    PlotSettingsForm * form = plotForms.value(name.isEmpty() ? currentPlotName : name, nullptr);
    if (form)
    {
        LazyNutJob *job = new LazyNutJob;
        job->logMode |= ECHO_INTERPRETER; // debug purpose
        job->cmdList = form->getSettingsCmdList();
        SessionManager::instance()->submitJobs(job);
    }
}

void PlotSettingsWindow::getPlotType()
{
    LazyNutJob *job = new LazyNutJob;
    job->cmdList = QStringList({QString("xml %1 description").arg(currentPlotName)});
    job->setAnswerReceiver(this, SLOT(extractPlotType(QDomDocument*)), AnswerFormatterType::XML);
    SessionManager::instance()->submitJobs(job);
}

void PlotSettingsWindow::extractPlotType(QDomDocument *description)
{
    currentPlotType = QFileInfo(XMLelement(*description)["Type"]()).fileName();
    plotTypeBox->setText(currentPlotType);
}

void PlotSettingsWindow::setCurrentPlotType(QString rScript)
{
//    qDebug() << "In setCurrentPlotType, rScript is" << rScript;
    currentPlotType = rScript;
    if (!currentPlotType.isEmpty())
    {
        QSettings settings("QtEasyNet", "nmConsole");
        QStringList rScripts = settings.value("recentRScripts","").toStringList();
        rScripts.removeAll(rScript);
        rScripts.prepend(rScript);
        while (rScripts.size() > MaxRecentRScripts)
            rScripts.removeLast();

        settings.setValue("recentRScripts", rScripts);
        updateRecentRScriptsActs();
    }
}




NewPlotWizard::NewPlotWizard(QWidget *parent)
    : QWizard(parent)
{
    addPage(new NewPlotPage);
    setWindowTitle(tr("New Plot"));
    setAttribute(Qt::WA_DeleteOnClose, true);
}

void NewPlotWizard::accept()
{
    emit newRPlotCreated(field("name").toString(), field("type").toString());
    QDialog::accept();
}


NewPlotPage::NewPlotPage(QWidget *parent)
    : QWizardPage(parent)
{
    setWindowTitle(tr("New Plot"));
    nameLabel = new QLabel("Plot name (no file extension):");
    nameEdit = new QLineEdit;
    typeLabel = new QLabel("Plot type (R script):");
    typeEdit = new QLineEdit;
    typeEdit->setReadOnly(true);
    browseButton = new QPushButton("Browse");
    connect(browseButton, SIGNAL(clicked()), this, SLOT(selectRScript()));
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(nameLabel, 0, 0);
    gridLayout->addWidget(nameEdit, 0, 1);
    gridLayout->addWidget(typeLabel, 1, 0);
    gridLayout->addWidget(typeEdit, 1, 1);
    gridLayout->addWidget(browseButton, 1, 2);
    setLayout(gridLayout);

    registerField("name*", nameEdit);
    registerField("type*", typeEdit);
}

void NewPlotPage::selectRScript()
{
    QSettings settings("QtEasyNet", "nmConsole");
    QString rScriptsHome = settings.value("easyNetHome","").toString().append("/bin/R-library/plots");
    QString rScript = QFileDialog::getOpenFileName(this,tr("Please select an R script"),
                                                   rScriptsHome,"*.R");
    if (!rScript.isEmpty())
        typeEdit->setText(QFileInfo(rScript).fileName());
}

void PlotSettingsWindow::setDefaultModelSetting(QString setting, QString value)
{
    PlotSettingsForm * form = qobject_cast<PlotSettingsForm*>(plotControlPanelScrollArea->widget());
    if (form)
        form->setDefaultModelSetting(setting, value);
}

QMap<QString, QString> PlotSettingsWindow::getSettings(QString plotName)
{
    if (!plotForms.contains(plotName))
    {
        qDebug() << "ERROR: PlotSettingsWindow::getSettings plotName does not match any settings form widget";
        return QMap<QString, QString>();
    }
    return plotForms[plotName]->getSettings();
}

void PlotSettingsWindow::newAspectRatio(QSize size)
{
    plotAspr_=double(size.width())/size.height();
}

void PlotSettingsWindow::removePlotSettings(QString name)
{
    if (plotForms.value(name) == plotControlPanelScrollArea->widget())
    {
        plotNameBox->setText("");
        plotTypeBox->setText("");
    }
    delete plotForms.value(name);
    plotForms.remove(name);
    plotTypes.remove(name);
}
