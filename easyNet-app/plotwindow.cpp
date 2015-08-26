
#include <QtWidgets>
#include <QSvgWidget>
#include <QDomDocument>
#include <QListView>
#include <QScrollArea>
#include <QMapIterator>
#include <QInputDialog>

#include "plotwindow.h"
#include "codeeditor.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"
#include "lazynutlistmenu.h"
#include "plotsettingsform.h"
#include "xmlelement.h"




PlotWindow::PlotWindow(QWidget *parent)
    : createNewPlotText("Create new plot"),
      openPlotSettingsText("Open plot settings"),
      savePlotSettingsText("Save plot settings"),
      savePlotSettingsAsText("Save plot setings as..."),
      plotSettingsForm(nullptr),
      QMainWindow(parent)
{
    createPlotControlPanel();
    setUnifiedTitleAndToolBarOnMac(true);
    createActions();
    createToolBars();

}

void PlotWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(refreshAct);
}


int PlotWindow::getValueFromByteArray(QByteArray ba, QString key)
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


void PlotWindow::createPlotControlPanel()
{
    // actions and menus
    plotsMenu = new LazyNutListMenu;
    menuBar()->addMenu(plotsMenu);
    plotsMenu->setTitle("Plots");
    plotsMenu->setGetListCmd("xml list rplot");
    connect(plotsMenu, SIGNAL(selected(QString)), this, SLOT(setPlot(QString)));
//    plotsMenu->prePopulate(QStringList({createNewPlotText, openPlotSettingsText, savePlotSettingsText, savePlotSettingsAsText}));
    plotsMenu->prePopulate(createNewPlotText);

    selectRScriptAct = new QAction(tr("&Select R script"), this);
    selectRScriptAct->setStatusTip(tr("Select an existing R script"));
    connect(selectRScriptAct, SIGNAL(triggered()), this, SLOT(selectRScript()));

    for (int i = 0; i < MaxRecentRScripts; ++i)
    {
        recentRScriptsActs[i] = new QAction(this);
        recentRScriptsActs[i]->setVisible(false);
        connect(recentRScriptsActs[i], SIGNAL(triggered()),
                this, SLOT(selectRecentRScript()));
    }

    typeMenu = menuBar()->addMenu(tr("Plot &type"));
    typeMenu->addAction(selectRScriptAct);
    separatorAct = typeMenu->addSeparator();
    for (int i = 0; i < MaxRecentRScripts; ++i)
        typeMenu->addAction(recentRScriptsActs[i]);
    updateRecentRScriptsActs();

    drawAct = new QAction(tr("&Draw"), this);
    connect(drawAct, SIGNAL(triggered()), this, SLOT(draw()));
    QToolBar *plotToolbar = addToolBar("");
    plotToolbar->addAction(drawAct);
    plotControlPanelScrollArea = new QScrollArea;
    plotControlPanelScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    plotControlPanelScrollArea->setWidgetResizable(true);
    setCentralWidget(plotControlPanelScrollArea);

}

void PlotWindow::openPlotSettings()
{
    QSettings settings("QtEasyNet", "nmConsole");
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open plot settings file"),
                                                    settings.value("plotSettingsDir").toString(), tr("Settings Files (*.xml)"));
    if (!fileName.isEmpty())
        loadSettings(fileName);
}

void PlotWindow::loadSettings(QString fileName)
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
    buildSettingsForm(domDoc);
    // ...
}

void PlotWindow::createActions()
{
    refreshAct = new QAction(QIcon(":/images/reload.png"), tr("&Refresh"), this);
    refreshAct->setShortcuts(QKeySequence::Refresh);
//    refreshAct->setStatusTip(tr("Refresh plot"));
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(sendDrawCmd()));
}

void PlotWindow::updateRecentRScriptsActs()
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




void PlotWindow::importHomonyms(QDomDocument *settingsList)
{
    XMLelement xml = XMLelement(*settingsList);
    foreach (QString label, xml.listLabels())
    {
        if (plotSettingsForm->listLabels().contains(label))
        {
            QString value = plotSettingsForm->value(label);
            if (value.isEmpty())
                xml[label]["value"].setValue(xml[label]["default"]());

            else
                xml[label]["value"].setValue(value);
        }
    }
}

void PlotWindow::sendDrawCmd()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode &= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({QString("%1 get").arg(currentPlot)});
    param->answerFormatterType = AnswerFormatterType::SVG;
    param->setAnswerReceiver(this, SLOT(displaySVG(QByteArray)));
    SessionManager::instance()->setupJob(param, sender());
}

void PlotWindow::displaySVG(QByteArray plotByteArray)
{
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

//    send signal to load the byte array into the plot
    emit plot(currentPlot, plotByteArray);

}

void PlotWindow::setPlot(QString name)
{
    qDebug() << "In setPlot, name is" << name;
    if (name == createNewPlotText)
        newPlot();
//    else if (name == openPlotSettingsText)
//        openPlotSettings();
//    else if (name == savePlotSettingsText)
//        savePlotSettings();
//    else if (name == savePlotSettingsAsText)
//        savePlotSettingsAs();
    else
    {
        currentPlot = name;
        getPlotType(this, SLOT(getSettingsXML()));
//        getSettingsXML();
    }
}

void PlotWindow::newPlot()
{
//    bool ok;
//    QString name = QInputDialog::getText(this, tr("New plot"), tr("Plot name:"), QLineEdit::Normal,
//                                         QString(), &ok);
//    if (ok && !name.isEmpty())
//    {
//        createNewPlot(name);
//        currentPlot = name;
//    }
    NewPlotWizard *plotWizard = new NewPlotWizard(this);
    connect(plotWizard, SIGNAL(createNewPlotOfType(QString,QString)),
            this, SLOT(createNewPlotOfType(QString,QString)));
    plotWizard->show();
}

void PlotWindow::createNewPlot(QString name)
{
    setCurrentPlotType(QString());
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({QString("create rplot %1").arg(name)});
    param->setNextJobReceiver(this,SLOT(getSettingsXML()));
    SessionManager::instance()->setupJob(param, sender());
}

void PlotWindow::createNewPlotOfType(QString name, QString rScript,
                                     QMap <QString,QString> _defaultSettings)
{
    defaultSettings = _defaultSettings;
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({
                                     QString("create rplot %1").arg(name),
                                     QString("%1 set_type %2").arg(name).arg(rScript)
                                 });
    param->setNextJobReceiver(this,SLOT(getSettingsXML()));
    SessionManager::instance()->setupJob(param, sender());
    currentPlot = name;
    currentPlotType = rScript;

}

void PlotWindow::setType(QString rScript)
{
    setCurrentPlotType(rScript);
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({QString("%1 set_type %2").arg(currentPlot).arg(rScript)});
    param->setNextJobReceiver(this,SLOT(getSettingsXML()));
    SessionManager::instance()->setupJob(param, sender());
    currentPlotType = rScript;
}

void PlotWindow::getSettingsXML()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode &= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({QString("xml %1 list_settings").arg(currentPlot)});
    param->answerFormatterType = AnswerFormatterType::XML;
    param->setAnswerReceiver(this, SLOT(buildSettingsForm(QDomDocument*)));
    SessionManager::instance()->setupJob(param, sender());
}


void PlotWindow::buildSettingsForm(QDomDocument *settingsList)
{
//    if (plotSettingsForm)
//        importHomonyms(settingsList);

    plotSettingsForm = new PlotSettingsForm(settingsList, currentPlot, defaultSettings, this);
    defaultSettings.clear();
    plotTitleLabel = new QLabel(QString("%1 (%2)").arg(currentPlot).arg(currentPlotType));
    plotTitleLabel->setStyleSheet("QLabel {"
                             "background-color: white;"
                             "border: 1px solid black;"
                             "padding: 4px;"
                             "font: bold 12pt;"
                             "}");

    QVBoxLayout *vboxLayout = new QVBoxLayout;
    vboxLayout->addWidget(plotTitleLabel);
    vboxLayout->addWidget(plotSettingsForm);
    vboxLayout->addStretch();
    plotSettingsWidget = new QWidget;
    plotSettingsWidget->setLayout(vboxLayout);
    plotSettingsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    plotControlPanelScrollArea->setWidget(plotSettingsWidget);

}

void PlotWindow::sendSettings(QObject *nextJobReceiver, const char *nextJobSlot)
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = plotSettingsForm->getSettingsCmdList();
    qDebug() << "PlotWindow::sendSettings"<< plotSettingsForm->getSettingsCmdList() << sender();
    param->setNextJobReceiver(nextJobReceiver, nextJobSlot);
    SessionManager::instance()->setupJob(param, sender());
}

void PlotWindow::getPlotType(QObject *nextJobReceiver, const char *nextJobSlot)
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode &= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({QString("xml %1 description").arg(currentPlot)});
    param->answerFormatterType = AnswerFormatterType::XML;
    param->setAnswerReceiver(this, SLOT(extractPlotType(QDomDocument*)));
    param->setNextJobReceiver(nextJobReceiver, nextJobSlot);
    SessionManager::instance()->setupJob(param, sender());
}

void PlotWindow::extractPlotType(QDomDocument *description)
{
    currentPlotType = QFileInfo(XMLelement(*description)["Type"]()).fileName();
}

//void PlotWindow::updateSettingsForm()
//{
//    sendSettings(this, SLOT(getSettingsXML()));
//}

void PlotWindow::draw()
{
    if (plotSettingsForm)
        sendSettings(this, SLOT(sendDrawCmd()));
    else
        SessionManager::instance()->setupNoOp(sender());
}



void PlotWindow::selectRScript()
{
    QSettings settings("QtEasyNet", "nmConsole");
    QString rScriptsHome = settings.value("easyNetHome","").toString().append("/bin/R-library/plots");
    QString rScript = QFileDialog::getOpenFileName(this,tr("Please select an R script"),
                                                   rScriptsHome,"*.R");
    if (!rScript.isEmpty())
        setType(QFileInfo(rScript).fileName());
}

void PlotWindow::selectRecentRScript()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        setType(action->data().toString());
}

void PlotWindow::setCurrentPlotType(QString rScript)
{
    qDebug() << "In setCurrentPlotType, rScript is" << rScript;
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
    emit createNewPlotOfType(field("name").toString(), field("type").toString());
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

void PlotWindow::setDefaultModelSetting(QString setting, QString value)
{
    plotSettingsForm->setDefaultModelSetting(setting, value);
}
