#include "trialwidget.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "xmlelement.h"
#include "mycombobox.h"
#include "sessionmanager.h"
#include "easyNetMainWindow.h"
#include "lazynutjob.h"
#include "tableviewer2.h"
#include "plotviewer.h"
#include "tablewindow.h"

#include <QComboBox>
#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>
#include <QDomDocument>
#include <QDebug>
#include <QToolButton>
#include <QAction>
#include <QLineEdit>

#include <memory>


Q_DECLARE_METATYPE(QDomDocument*)


TrialWidget::TrialWidget(QWidget *parent)
    : runAllMode(false), QWidget(parent)
{
    layout = new QHBoxLayout;
    layout1 = new QHBoxLayout;
    layout2 = new QHBoxLayout;
    layout3 = new QVBoxLayout;



    trialFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    trialFilter->setType("trial");
    trialDescriptionUpdater = new ObjectUpdater(this);
    trialDescriptionUpdater->setProxyModel(trialFilter);
    connect(trialDescriptionUpdater,SIGNAL (objectUpdated(QDomDocument*, QString)),
            this,SLOT(buildComboBoxes(QDomDocument*)));
    connect(trialDescriptionUpdater,SIGNAL (objectUpdated(QDomDocument*, QString)),
            this,SIGNAL(trialDescriptionUpdated(QDomDocument*)));


    runAction = new QAction(QIcon(":/images/media-play-8x.png"),tr("&Run"), this);
    runAction->setStatusTip(tr("Run"));
    connect(runAction,SIGNAL(triggered()),this,SLOT(runTrial()));
    runButton = new QToolButton(this);
    runButton->hide();

    hideSetComboBoxAction = new QAction(QIcon(":/images/icon_dismiss.png"),tr("&Hide"), this);
    hideSetComboBoxAction->setStatusTip(tr("Hide"));
    connect(hideSetComboBoxAction,SIGNAL(triggered()),this,SLOT(hideSetComboBox()));

    buildComboBoxesTest(QStringList());
}

TrialWidget::~TrialWidget()
{
}

void TrialWidget::update(QString trialName)
{
//    trialDescriptionUpdater->requestDescription(trialName); // 3/7/15 -> added cos currently no descriptions for trials
//    qDebug() << "Entered trialwidget update" << trialName;
    if (trialName.isEmpty())
        return;
    trialFilter->setName(trialName);
//    qDebug() << "called setName";

}

void TrialWidget::buildComboBoxes(QDomDocument* domDoc)
{
    QStringList argList;
    defs.clear();
    XMLelement arg = XMLelement(*domDoc)["arguments"].firstChild();
    while (!arg.isNull())
    {
        argList.append(arg.label());
        defs[arg.label()]=(arg.value());
        arg = arg.nextSibling();
    }
    qDebug() << "buildComboBoxes args = " << argList << "defs = " << defs;
    if (argList.size())
        buildComboBoxesTest(argList);

}

void TrialWidget::buildComboBoxesTest(QStringList args)
{
    // first delete existing labels/boxes/button
    clearLayout(layout);

    // need to reconstruct constituent layouts that were in layout
    layout1 = new QHBoxLayout;
    layout2 = new QHBoxLayout;
    layout3 = new QVBoxLayout;

    argumentMap.clear();
    labelList.clear();

    setComboBox = new QComboBox(this);
    setComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    setCancelButton = new QToolButton(this);
    setCancelButton->setIcon(QIcon(":/images/icon_dismiss.png"));
    setCancelButton->setAutoRaise(true);
    setCancelButton->setDefaultAction(hideSetComboBoxAction);
    layout1->addWidget(setComboBox);
    layout1->addWidget(setCancelButton);

    // now add new boxes
    for (int i=0;i<args.count();i++)
    {
        argumentMap[args[i]] = new myComboBox(this); // new QComboBox(this);
        argumentMap[args[i]]->setArg(args[i]);
        argumentMap[args[i]]->setEditable(true);
        argumentMap[args[i]]->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        argumentMap[args[i]]->setMinimumSize(100, argumentMap[args[i]]->minimumHeight());
        connect(argumentMap[args[i]], SIGNAL(editTextChanged(QString)),this,SLOT(setRunButtonIcon()));
        connect(argumentMap[args[i]], SIGNAL(argWasChanged(QString)),this,SLOT(argWasChanged(QString)));

        labelList.push_back(new QLabel(args[i]+":"));
        layout2->addWidget(labelList[i]);
        layout2->addWidget(argumentMap[args[i]]);
    }

    if (!args.isEmpty())
    {
        QLineEdit *ed = argumentMap[args[args.count()-1]]->lineEdit();
        connect(ed, SIGNAL(returnPressed()),runAction,SIGNAL(triggered()));
    }

    if (runButton == NULL)
        runButton = new QToolButton(this);
    runButton->setAutoRaise(true);
    runButton->setDefaultAction(runAction);
    runButton->setIcon(QIcon(":/images/run_disabled.png"));
    runButton->setIconSize(QSize(40, 40));
    runButton->show();

    layout3->addLayout(layout1);
    layout3->addLayout(layout2);

    layout->addLayout(layout3);
    layout->addWidget(runButton);
    setLayout(layout);

    hideSetComboBox();
    clearArgumentBoxes();
}

void TrialWidget::argWasChanged(QString arg)
{
    argChanged = arg;
}

void TrialWidget::clearLayout(QLayout *layout)
{
    QLayoutItem *item;
    while((item = layout->takeAt(0)))
    {
        if (item->layout())
        {
            clearLayout(item->layout());
//            delete item->layout();
        }
        if (item->widget())
        {
            if (item->widget() != runButton)
                delete item->widget();
        }
        delete item;
    }


}

QString TrialWidget::getTrialCmd()
{
    QString cmd;
    QMap<QString, myComboBox*>::const_iterator i = argumentMap.constBegin();
    while (i != argumentMap.constEnd())
    {
        cmd += " ";
        cmd += i.key();
        cmd += "=";
        cmd += static_cast<myComboBox*>(argumentMap[i.key()])->currentText();
        ++i;
    }
    return (cmd);
}

QStringList TrialWidget::getArguments()
{
    return(argumentMap.keys());
}

void TrialWidget::runTrial()
{
    if (SessionManager::instance()->currentTrial().isEmpty())
    {
        MainWindow::instance()->msgBox("Choose which type of trial to run");
        return;
    }
    if (SessionManager::instance()->currentModel().isEmpty())
    {
        QMessageBox::warning(this, "Help", "Choose which model to run");
        return;
    }
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    if (runAllMode)
        runTrialList(job);
    else
        runSingleTrial(job);

    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->updateObjectCatalogueJobs();
    QDomDocument *trialRunInfo = createTrialRunInfo(); // will be a smart pointer
    QMap<QString, QVariant> data;
    data.insert("trialRunInfo", QVariant::fromValue(trialRunInfo));
    jobs.last()->data = data;
    jobs.last()->appendEndOfJobReceiver(MainWindow::instance()->tableWindow, SLOT(dispatch()));
    jobs.last()->appendEndOfJobReceiver(MainWindow::instance()->plotViewer, SLOT(dispatch()));

    if (runAllMode)
    {
        MainWindow::instance()->runAllTrialMsgAct->setVisible(true);
        jobs.last()->appendEndOfJobReceiver(MainWindow::instance(), SIGNAL(runAllTrialEnded()));
    }
    SessionManager::instance()->submitJobs(jobs);
}

QString TrialWidget::defaultDataframe()
{
    return QString("((%1 default_observer) default_dataframe)")
            .arg(SessionManager::instance()->currentTrial());
}

void TrialWidget::runSingleTrial(LazyNutJob *job)
{

    job->cmdList << QString("%1 clear").arg(defaultDataframe());

    // case hack to avoid problems with UPPERCASE!!!
    QString trialString = getTrialCmd().toLower();
    job->cmdList << QString("%1 %2 step %3")
            .arg(MainWindow::instance()->quietMode)
            .arg(SessionManager::instance()->currentTrial())
            .arg(trialString);


//    QString displayTableName = SessionManager::instance()->currentTrial();
//    displayTableName.append(".table");

    // now rbind the data to existing trial table
//    job->cmdList << QString("R << eN[\"%1\"] <- rbind(eN[\"%1\"],eN[\"%2\"])")
//            .arg(displayTableName)
//            .arg(tableName);

    // display table of means after running set
//    job->cmdList << QString("xml %1 get").arg(displayTableName);
//    job->setAnswerReceiver(MainWindow::instance()->tablesWindow, SLOT(prepareToAddDataFrameToWidget(QDomDocument*, QString)), AnswerFormatterType::XML);



//    QMap<QString, QVariant> headerReplace;
//    headerReplace.insert(SessionManager::instance()->currentTrial(), "");
//    headerReplace.insert("event_pattern", "");
//    headerReplace.insert("\\(", "");
//    headerReplace.insert("\\)", "");
//    job->data = headerReplace;


//    MainWindow::instance()->tablesWindow->switchTab(displayTableName);
}

QDomDocument * TrialWidget::createTrialRunInfo()
{
    QDomDocument *trialRunInfo = new QDomDocument();
    QDomElement rootElem = trialRunInfo->createElement("string");
    rootElem.setAttribute("value", "Trial run info");
    trialRunInfo->appendChild(rootElem);
    QDomElement trialElem = trialRunInfo->createElement("string");
    trialElem.setAttribute("label", "Trial");
    trialElem.setAttribute("value", SessionManager::instance()->currentTrial());
    rootElem.appendChild(trialElem);
    QDomElement modeElem = trialRunInfo->createElement("string");
    modeElem.setAttribute("label", "Run mode");
    modeElem.setAttribute("value", runAllMode ? "list" : "single");
    rootElem.appendChild(modeElem);
    QDomElement dataframeElem = trialRunInfo->createElement("object");
    dataframeElem.setAttribute("label", "Results");
    dataframeElem.setAttribute("value", QString("((%1 default_observer) default_dataframe)")
                               .arg(SessionManager::instance()->currentTrial()
                               ));
    rootElem.appendChild(dataframeElem);
    QDomElement valuesElem = trialRunInfo->createElement("map");
    trialElem.setAttribute("label", "Values");
    QMapIterator<QString, myComboBox*> argumentMap_it(argumentMap);
    while(argumentMap_it.hasNext())
    {
        argumentMap_it.next();
        QDomElement valueElem = trialRunInfo->createElement("string");
        valueElem.setAttribute("label", argumentMap_it.key());
        valueElem.setAttribute("value", static_cast<myComboBox*>(argumentMap[argumentMap_it.key()])->currentText());
        valuesElem.appendChild(valueElem);
    }
    rootElem.appendChild(valuesElem);

    return trialRunInfo; // will be a smart pointer
}

void TrialWidget::runTrialList(LazyNutJob *job)
{

    job->cmdList << QString("%1 clear").arg(defaultDataframe());
    job->cmdList << QString("set %1").arg(getTrialCmd());
    job->cmdList << QString("%1 %2 run_set %3")
            .arg(MainWindow::instance()->quietMode)
            .arg(SessionManager::instance()->currentTrial())
            .arg(getStimulusSet());
    job->cmdList << QString("unset %1").arg(getArguments().join(" "));
//    QString displayTableName = MainWindow::instance()->tablesWindow->addTable();
//    job->cmdList << QString("%1 copy %2").arg(tableName).arg(displayTableName);
//      // display table of means after running set
//    job->cmdList << QString("xml %1 get").arg(displayTableName);
//    job->setAnswerReceiver(MainWindow::instance()->tablesWindow, SLOT(prepareToAddDataFrameToWidget(QDomDocument*, QString)), AnswerFormatterType::XML);
//    job->appendEndOfJobReceiver(MainWindow::instance()->tablesWindow, SLOT(setPrettyHeaderFromJob()));
//    job->appendEndOfJobReceiver(MainWindow::instance(), SIGNAL(runAllTrialEnded()));

//    QMap<QString, QVariant> headerReplace;
//    headerReplace.insert(SessionManager::instance()->currentTrial(), "");
//    headerReplace.insert("event_pattern", "");
//    headerReplace.insert("\\(", "");
//    headerReplace.insert("\\)", "");
//    job->data = headerReplace;

//    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
//            << job
//            << SessionManager::instance()->updateObjectCatalogueJobs();

//    // write info XML and append it to the last job, append receivers
//    QDomDocument *trialRunInfo = createTrialRunInfo();
////    qDebug() << trialRunInfo->toString();
//    // append to last job
//    jobs.last()->data = QVariant::fromValue(trialRunInfo);



//    SessionManager::instance()->submitJobs(jobs);


//    MainWindow::instance()->resultsDock->raise();
//    MainWindow::instance()->resultsPanel->setCurrentIndex(MainWindow::instance()->outputTablesTabIdx);
}

bool TrialWidget::checkIfReadyToRun()
{
    QMap<QString, myComboBox*>::const_iterator i = argumentMap.constBegin();
    while (i != argumentMap.constEnd())
    {
        if (static_cast<myComboBox*>(argumentMap[i.key()])->currentText().isEmpty())
            return false;
        i++;
    }
    return true;
}

void TrialWidget::clearArgumentBoxes()
{
    QMap<QString, myComboBox*>::const_iterator i = argumentMap.constBegin();

    while (i != argumentMap.constEnd())
    {
        static_cast<myComboBox*>(argumentMap[i.key()])->clearEditText();
        static_cast<myComboBox*>(argumentMap[i.key()])->setCurrentText(defs[i.key()]);
        i++;
    }
}

void TrialWidget::clearDollarArgumentBoxes()
{
    QMap<QString, myComboBox*>::const_iterator i = argumentMap.constBegin();
    while (i != argumentMap.constEnd())
    {
        qDebug()<<static_cast<myComboBox*>(argumentMap[i.key()])->currentText();
        if(!static_cast<myComboBox*>(argumentMap[i.key()])->currentText().isEmpty()) // fix Jamesbug
            if (static_cast<myComboBox*>(argumentMap[i.key()])->currentText().at(0)=='$')
        {
            static_cast<myComboBox*>(argumentMap[i.key()])->clearEditText();
            static_cast<myComboBox*>(argumentMap[i.key()])->setCurrentText(defs[i.key()]);
        }
        i++;
    }
}

void TrialWidget::setRunButtonIcon()
{
    if (checkIfReadyToRun())
    {
        runButton->setIcon(QIcon(":/images/run_enabled.png"));
        runButton->setEnabled(true);
    }
    else
    {
        runButton->setIcon(QIcon(":/images/run_disabled.png"));
        runButton->setEnabled(false);
    }

}

void TrialWidget::hideSetComboBox()
{
    setComboBox->hide();
    setCancelButton->hide();
//    clearArgumentBoxes();
    clearDollarArgumentBoxes();
    runAllMode = false;
//    emit runAllModeChanged(false);

}

void TrialWidget::showSetComboBox()
{
    setComboBox->show();
    setCancelButton->show();
    runAllMode = true;
//    emit runAllModeChanged(true);
}

void TrialWidget::showSetLabel(QString set)
{
    qDebug() << "Entered showsetLabel";
    showSetComboBox();
    setComboBox->addItem(set);
    setComboBox->setCurrentIndex(setComboBox->findData(set,Qt::DisplayRole));
    SessionManager::instance()->setCurrentSet(set);
}

void TrialWidget::restoreComboBoxText()
{
    qDebug() << "Entered restoreComboBoxText";
    if (argChanged.isEmpty())
        return;
    if (argumentMap.isEmpty())
        return;
    myComboBox* box = static_cast<myComboBox*>(argumentMap[argChanged]);
    qDebug() << "combobox is" << box;
    qDebug() << "combobox text should be" << box->savedComboBoxText;
    box->restoreComboBoxText();

}

QString TrialWidget::getStimulusSet()
{
    return setComboBox->currentText();
}
