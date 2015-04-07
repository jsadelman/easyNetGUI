/****************************************************************************
**
**
****************************************************************************/

#include <QtWidgets>
#include <QSvgWidget>
#include <QDomDocument>
#include <QListView>
#include <QScrollArea>

#include "plotwindow.h"
#include "codeeditor.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"
#include "plotsettingsbaseeditor.h"
#include "plotsettingsdelegate.h"
#include "plotsettingsmodel.h"


NumericSettingsForPlotWidget::NumericSettingsForPlotWidget(QString name, QString value, QString comment, QString defaultValue, QWidget *parent)
    : name(name), value(value), comment(comment), defaultValue(defaultValue), QGroupBox(parent)
{
     QHBoxLayout *layout = new QHBoxLayout;
     nameLabel = new QLabel(name, this);
     commentButton = new QPushButton("?", this);
     connect(commentButton, SIGNAL(clicked()), this, SLOT(displayComment()));
     // need to take care of int or real, using QDoubleSpinBox  for the latter
     // this is a bad implementation
     if (defaultValue.contains("."))
     {
         doubleSpinBox = new QDoubleSpinBox(this);
         if (value.isEmpty())
             doubleSpinBox->setValue(defaultValue.toDouble());
         else
             doubleSpinBox->setValue(value.toDouble());
         connect(doubleSpinBox, SIGNAL(valueChanged(QString)), this, SLOT(setValue(QString)));
     }
     else
     {
         intSpinBox = new QSpinBox(this);
         if (value.isEmpty())
             intSpinBox->setValue(defaultValue.toInt());
         else
             intSpinBox->setValue(value.toInt());
         connect(intSpinBox, SIGNAL(valueChanged(QString)), this, SLOT(setValue(QString)));
     }

     layout->addWidget(nameLabel);
     layout->addWidget(commentButton);
     layout->addStretch();
     if (defaultValue.contains("."))
        layout->addWidget(doubleSpinBox);
     else
         layout->addWidget(intSpinBox);
     setLayout(layout);

}

void NumericSettingsForPlotWidget::displayComment()
{
    QMessageBox::information(this, "Setting description", comment);
}

// loads of code duplication here
FactorSettingsForPlotWidget::FactorSettingsForPlotWidget(QString name, QString value, QString comment, QString defaultValue, QWidget *parent)
    : name(name), value(value), comment(comment), defaultValue(defaultValue), QGroupBox(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    // header
    QWidget *headerBox = new QWidget(this);
    QHBoxLayout *headerBoxLayout = new QHBoxLayout;
    QLabel *nameLabel = new QLabel(name, this);
    QPushButton *commentButton = new QPushButton("?", this);
    connect(commentButton, SIGNAL(clicked()), this, SLOT(displayComment()));
    headerBoxLayout->addWidget(nameLabel);
    headerBoxLayout->addWidget(commentButton);
    headerBoxLayout->addStretch();
    headerBox->setLayout(headerBoxLayout);
    // main box
    QWidget *mainBox = new QWidget(this);
    QGridLayout *mainBoxLayout = new QGridLayout;
    sourceListView = new QListView(this);
    destinationListView = new QListView(this);
    QPushButton *addButton = new QPushButton("==>", this);
    QPushButton *subtractButton = new QPushButton("<==", this);
    mainBoxLayout->addWidget(sourceListView, 0,0,4,1);
    mainBoxLayout->addWidget(destinationListView, 0,2,4,1);
    mainBoxLayout->addWidget(addButton,1,1);
    mainBoxLayout->addWidget(subtractButton,2,1);
    mainBoxLayout->setColumnStretch(0,10);
    mainBoxLayout->setColumnStretch(2,10);
    mainBox->setLayout(mainBoxLayout);

    layout->addWidget(headerBox);
    layout->addWidget(mainBox);
    setLayout(layout);
}

void FactorSettingsForPlotWidget::displayComment()
{
    QMessageBox::information(this, "Setting description", comment);
}






PlotWindow::PlotWindow(QWidget *parent)
    : QMainWindow(parent)

{
    plot_svg = new QSvgWidget(this);
    setCentralWidget(plot_svg);

    QFile file(":/images/test.svg");
     if (file.open(QIODevice::ReadOnly))
        displaySVG(file.readAll());

    createPlotControlPanel();
//    createActions();
//    createMenus();
//    createToolBars();
//    createStatusBar();

//    readSettings();

/*
 *     connect(textEdit->document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));
*/
//    setCurrentFile("");
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

    selectOutputAct = new QAction(tr("&Select output"), this);
    selectOutputAct->setStatusTip(tr("Select a model output to plot"));
    connect(selectOutputAct, SIGNAL(triggered()), this, SLOT(selectOutput()));

    typeMenu = menuBar()->addMenu(tr("Plot &type"));
    typeMenu->addAction(selectRScriptAct);
    separatorAct = typeMenu->addSeparator();
    for (int i = 0; i < MaxRecentRScripts; ++i)
        typeMenu->addAction(recentRScriptsActs[i]);
    updateRecentRScriptsActs();


    plotControlPanelWindow = new QMainWindow(this);
    redrawAct = new QAction(tr("&Redraw"), this);
    connect(redrawAct, SIGNAL(triggered()), this, SLOT(redraw()));
    QToolBar *plotToolbar = plotControlPanelWindow->addToolBar("");
    plotToolbar->addAction(redrawAct);
    plotControlPanelScrollArea = new QScrollArea;
    plotControlPanelScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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

/*
      openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
//    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));


    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
//    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
//    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
//    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
//                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));

*/
}

void PlotWindow::updateRecentRScriptsActs()
{
    QSettings settings("QtEasyNet", "nmConsole");
    QStringList rScripts = settings.value("recentRScripts","").toStringList();
    int numRecentRScripts = qMin(rScripts.size(), (int)MaxRecentRScripts);
    for (int i = 0; i < numRecentRScripts; ++i)
    {
        QString fileName = QFileInfo(rScripts[i]).fileName();
        QString text = tr("&%1 %2").arg(i + 1).arg(fileName);
        recentRScriptsActs[i]->setText(text);
        recentRScriptsActs[i]->setData(fileName);
        recentRScriptsActs[i]->setVisible(true);
    }
    for (int i = numRecentRScripts; i < MaxRecentRScripts; ++i)
        recentRScriptsActs[i]->setVisible(false);

        separatorAct->setVisible(numRecentRScripts > 0);
}

/*
void plotWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    if (cutAllowed)
        editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    if (pasteAllowed)
        editMenu->addAction(pasteAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}
*/

void PlotWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(refreshAct);
//        fileToolBar->addAction(openAct);
//    fileToolBar->addAction(saveAct);

//    editToolBar = addToolBar(tr("Edit"));
//    editToolBar->addAction(copyAct);
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

void PlotWindow::listSettings()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = {"xml plo list_settings"};
    param->answerFormatterType = AnswerFormatterType::XML;
    param->setAnswerReceiver(this, SLOT(buildPlotControlPanel(QDomDocument*)));
    SessionManager::instance()->setupJob(param, sender());
}

//void PlotWindow::buildPlotControlPanel(QDomDocument *settingsList)
//{
//    numericSettingsWidgets.clear();
//    QWidget *plotControlPanel = new QWidget(this);
//    plotControlPanelLayout = new QVBoxLayout;
//    QString name, type, value, comment, defaultValue, label;

//    QDomNode settingNode = settingsList->firstChild().firstChild();
//    while (!settingNode.isNull())
//    {
//        if (settingNode.isElement())
//            name = settingNode.toElement().attribute("label");
//            QDomNode labelValueNode = settingNode.firstChild();
//            while (!labelValueNode.isNull())
//            {
//                label = labelValueNode.toElement().attribute("label");
//                if (label == "value")
//                    value = labelValueNode.toElement().attribute("value");
//                else if (label == "type")
//                    type = labelValueNode.toElement().attribute("value");
//                else if (label == "comment")
//                    comment = labelValueNode.toElement().attribute("value");
//                else if (label == "default")
//                    defaultValue = labelValueNode.toElement().attribute("value");
//                labelValueNode = labelValueNode.nextSibling();
//            }
//        if (type == "numeric")
//        {
//            NumericSettingsForPlotWidget *settingsWidget = new NumericSettingsForPlotWidget(name, value, comment, defaultValue, this);
//            plotControlPanelLayout->addWidget(settingsWidget);
//            numericSettingsWidgets.append(settingsWidget);
//        }
//        else if (type == "free")
//        {
//            FactorSettingsForPlotWidget *settingsWidget = new FactorSettingsForPlotWidget(name, value, comment, defaultValue, this);
//            plotControlPanelLayout->addWidget(settingsWidget);
//            //numericSettingsWidgets.append(settingsWidget); // just settingsWidgets should be fine
//        }
//        settingNode = settingNode.nextSibling();
//    }
//    plotControlPanel->setLayout(plotControlPanelLayout);
//    plotControlPanelScrollArea->setWidget(plotControlPanel);
//}

void PlotWindow::buildPlotControlPanel(QDomDocument *settingsList)
{
    plotSettingsModel = new PlotSettingsModel(settingsList, this);
    plotSettingsView = new QListView(this);
    plotSettingsDelegate = new PlotSettingsDelegate(this);
    plotSettingsView->setModel(plotSettingsModel);
    plotSettingsView->setItemDelegate(plotSettingsDelegate);
    plotControlPanelScrollArea->setWidget(plotSettingsView);
}

void PlotWindow::redraw()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    foreach (NumericSettingsForPlotWidget *settingsWidget, numericSettingsWidgets)
    {
        param->cmdList.append(QString("plo setting %1 %2").arg(settingsWidget->getName()).arg(settingsWidget->getValue()));
    }
    param->cmdList.append("plo draw output");
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
//    rScripts.clear(); // remove this
    rScripts.removeAll(rScript);
    rScripts.prepend(rScript);
    while (rScripts.size() > MaxRecentRScripts)
        rScripts.removeLast();

    settings.setValue("recentRScripts", rScripts);
    updateRecentRScriptsActs();

}

void PlotWindow::selectOutput()
{

}

