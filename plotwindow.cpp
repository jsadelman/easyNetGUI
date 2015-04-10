
#include <QtWidgets>
#include <QSvgWidget>
#include <QDomDocument>
#include <QListView>
#include <QScrollArea>
#include <QMapIterator>

#include "plotwindow.h"
#include "codeeditor.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"
#include "lazynutlistmenu.h"
#include "plotsettingsform.h"



PlotWindow::PlotWindow(QWidget *parent)
    : QMainWindow(parent)

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

    dataMenu = new LazyNutListMenu;
    menuBar()->addMenu(dataMenu);
    dataMenu->setTitle("Data");
    dataMenu->setGetListCmd("xml list dataframe");
    connect(dataMenu, SIGNAL(selected(QString)), this, SLOT(setDataframe(QString)));


    plotControlPanelWindow = new QMainWindow(this);
    redrawAct = new QAction(tr("&Redraw"), this);
    connect(redrawAct, SIGNAL(triggered()), this, SLOT(redraw()));
    QToolBar *plotToolbar = plotControlPanelWindow->addToolBar("");
    plotToolbar->addAction(redrawAct);
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
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refreshSvg()));
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

void PlotWindow::refreshSvg()
{
// This function sends a "plo get" command to the inpterpreter,
// reads the output into a byteArray, does some processing on it
// then loads it into the svgWidget

    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = {"plo get"};
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

void PlotWindow::setType(QString rScript)
{
    // disable all that is not activity.R
    if (!rScript.endsWith("activity.R"))
        return;
    setCurrentPlotType(rScript);
//    currentPlotType = rScript;
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = {QString("plo set_type %1").arg(rScript)};
    param->setNextJobReceiver(this,SLOT(listSettings()));
    SessionManager::instance()->setupJob(param);
    // probably it's possible to condense the two jobs into one
    // since set_type gives no answer.
}

void PlotWindow::setDataframe(QString dataframe)
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList =  {
        QString("xml %1 get_colname_dimensions").arg(dataframe),
        QString("plo set_data %1").arg(dataframe)
    };
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(plotSettingsForm, SLOT(setFactorList(QStringList)));
    SessionManager::instance()->setupJob(param);
}

void PlotWindow::listSettings()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = {"xml plo list_settings"};
    param->answerFormatterType = AnswerFormatterType::XML;
    param->setAnswerReceiver(this, SLOT(buildPlotControlPanel(QDomDocument*)));
    SessionManager::instance()->setupJob(param, sender());
}


void PlotWindow::buildPlotControlPanel(QDomDocument *settingsList)
{
    plotSettingsForm = new PlotSettingsForm(settingsList, this);
    plotControlPanelScrollArea->setWidget(plotSettingsForm);
}

void PlotWindow::redraw()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    QMapIterator<QString,QString> settingsIter(plotSettingsForm->getSettings());
    while (settingsIter.hasNext())
    {
        settingsIter.next();
        param->cmdList.append(QString("plo setting %1 %2").arg(settingsIter.key()).arg(settingsIter.value()));
    }
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->setNextJobReceiver(this,SLOT(refreshSvg()));
    SessionManager::instance()->setupJob(param,sender());
}



void PlotWindow::selectRScript()
{
    QSettings settings("QtEasyNet", "nmConsole");
    QString rScriptsHome = settings.value("easyNetHome","").toString().append("/bin/R-library/plots");
    QString rScript = QFileDialog::getOpenFileName(this,tr("Please select an R script"),
                                                   rScriptsHome,"*.R");
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


