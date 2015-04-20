
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



PlotWindow::PlotWindow(QWidget *parent)
    : createNewPlotText("Create new plot"), QMainWindow(parent)
{
    plot_svg = new QSvgWidget(this);
    setCentralWidget(plot_svg);

    createPlotControlPanel();
    setUnifiedTitleAndToolBarOnMac(true);

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


    plotControlPanelWindow = new QMainWindow(this);
    drawAct = new QAction(tr("&Draw"), this);
    connect(drawAct, SIGNAL(triggered()), this, SLOT(draw()));
    QToolBar *plotToolbar = plotControlPanelWindow->addToolBar("");
    plotToolbar->addAction(drawAct);
    plotControlPanelScrollArea = new QScrollArea;
    plotControlPanelScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    plotControlPanelScrollArea->setWidgetResizable(true);
    plotControlPanelWindow->setCentralWidget(plotControlPanelScrollArea);

    // dock plotControlPanel to the left of plot_svg
    QDockWidget *dockPlotControlPanel = new QDockWidget("Plot Control Panel", this);
    dockPlotControlPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockPlotControlPanel->setWidget(plotControlPanelWindow);
    addDockWidget(Qt::LeftDockWidgetArea, dockPlotControlPanel);
    dockPlotControlPanel->setMinimumWidth(400);

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


void PlotWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(refreshAct);
}

void PlotWindow::sendDrawCmd()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
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
    plot_svg->setMaximumSize(size);
    plot_svg->setMinimumSize(size);

//    load the byte array into the plot
    plot_svg->load(plotByteArray);

}

void PlotWindow::dumpSVG(QString svg)
{
    qDebug() << svg;
}

void PlotWindow::setPlot(QString name)
{
    if (name == createNewPlotText)
        newPlot();

    else
    {
        currentPlot = name;
        getSettingsXML();
    }
}

void PlotWindow::newPlot()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("New plot"), tr("Plot name:"), QLineEdit::Normal,
                                         QString(), &ok);
    if (ok && !name.isEmpty())
    {
        createNewPlot(name);
        currentPlot = name;
    }
}

void PlotWindow::createNewPlot(QString name)
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({QString("create rplot %1").arg(name)});
    SessionManager::instance()->setupJob(param, sender());
}

void PlotWindow::setType(QString rScript)
{
    setCurrentPlotType(rScript);
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({QString("%1 set_type %2").arg(currentPlot).arg(rScript)});
    param->setNextJobReceiver(this,SLOT(getSettingsXML()));
    SessionManager::instance()->setupJob(param, sender());
}

void PlotWindow::getSettingsXML()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({QString("xml %1 list_settings").arg(currentPlot)});
    param->answerFormatterType = AnswerFormatterType::XML;
    param->setAnswerReceiver(this, SLOT(buildSettingsForm(QDomDocument*)));
    SessionManager::instance()->setupJob(param, sender());
}


void PlotWindow::buildSettingsForm(QDomDocument *settingsList)
{
    plotSettingsForm = new PlotSettingsForm(settingsList, currentPlot, this);
    connect(plotSettingsForm, SIGNAL(updateRequest()), this, SLOT(updateSettingsForm()));
    plotControlPanelScrollArea->setWidget(plotSettingsForm);
}

void PlotWindow::sendSettings(QObject *nextJobReceiver, const char *nextJobSlot)
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = plotSettingsForm->getSettingsCmdList();
    param->setNextJobReceiver(nextJobReceiver, nextJobSlot);
    SessionManager::instance()->setupJob(param, sender());
}

void PlotWindow::updateSettingsForm()
{
    sendSettings(this, SLOT(getSettingsXML()));
}

void PlotWindow::draw()
{
    sendSettings(this, SLOT(sendDrawCmd()));
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
    currentPlotType = rScript;
    QSettings settings("QtEasyNet", "nmConsole");
    QStringList rScripts = settings.value("recentRScripts","").toStringList();
    rScripts.removeAll(rScript);
    rScripts.prepend(rScript);
    while (rScripts.size() > MaxRecentRScripts)
        rScripts.removeLast();

    settings.setValue("recentRScripts", rScripts);
    updateRecentRScriptsActs();

}


