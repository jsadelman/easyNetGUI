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
#include "xmlaccessor.h"

#include <QComboBox>
#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>
#include <QDomDocument>
#include <QDebug>
#include <QToolButton>
#include <QAction>
#include <QLineEdit>
#include <QIntValidator>
#include <QMessageBox>
#include <QCheckBox>

#include <memory>


Q_DECLARE_METATYPE(QDomDocument*)


TrialWidget::TrialWidget(QWidget *parent)
    : runAllMode(false), askDisableObserver(true), suspendingObservers(true), QWidget(parent)
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

    modelFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    connect(SessionManager::instance(), SIGNAL(currentModelChanged(QString)),
            modelFilter, SLOT(setName(QString)));

    modelDescriptionUpdater = new ObjectUpdater(this);
    modelDescriptionUpdater->setProxyModel(modelFilter);
    connect(modelDescriptionUpdater,SIGNAL(objectUpdated(QDomDocument*, QString)),
           this, SLOT(updateModelStochasticity(QDomDocument*)));


    runAction = new QAction(QIcon(":/images/media-play-8x.png"),tr("&Run"), this);
    runAction->setStatusTip(tr("Run"));
    connect(runAction,SIGNAL(triggered()),this,SLOT(runTrial()));
    runButton = new QToolButton(this);
    runButton->hide();

    hideSetComboBoxAction = new QAction(QIcon(":/images/icon_dismiss.png"),tr("&Hide"), this);
    hideSetComboBoxAction->setStatusTip(tr("Hide"));
    connect(hideSetComboBoxAction,SIGNAL(triggered()),this,SLOT(hideSetComboBox()));

    buildComboBoxesTest(QStringList());
    enabledObservers.clear();

    disableObserversMsg = new QMessageBox(
                QMessageBox::Question,
                "Suspend layer activity recording",
                "You are about to run a list of trials while layer activity is being recorded and displayed in plots. "
                "This may slow down the simulation.\n"
                "Do you want to suspend activity recording while running a list of trials?",
                QMessageBox::Yes | QMessageBox::No,
                this);
    dontAskAgainDisableObserverCheckBox = new QCheckBox("don't show this message again");
    disableObserversMsg->setCheckBox(dontAskAgainDisableObserverCheckBox);
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

    // Widget for stochastic models
    repetitionsLabel = new QLabel("Repetitions");
    repetitionsBox = new QComboBox(this);
    repetitionsBox->setEditable(true);
    repetitionsBox->setValidator(new QIntValidator(this));
    strategyLabel = new QLabel("Strategy");
    strategyBox = new QComboBox(this);
    strategyBox->addItems(QStringList({"ABAB", "AABB"}));
    strategyBox->setEditable(false);

    layout1->addWidget(repetitionsLabel);
    layout1->addWidget(repetitionsBox);
    layout1->addWidget(strategyLabel);
    layout1->addWidget(strategyBox);

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

QString TrialWidget::getStochasticCmd()
{
    return QString("%1 %2")
            .arg(strategyBox->currentText())
            .arg(repetitionsBox->currentText().isEmpty() ? "1" : repetitionsBox->currentText());
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
    emit aboutToRunTrial(trialRunInfo);
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
    job->cmdList << QString("%1 %2 step %3")
            .arg(MainWindow::instance()->quietMode)
            .arg(SessionManager::instance()->currentTrial())
            .arg(getTrialCmd());


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
    if (askDisableObserver)
    {
        int answer = disableObserversMsg->exec();
        suspendingObservers = answer == QMessageBox::Yes;
        askDisableObserver = dontAskAgainDisableObserverCheckBox->checkState() == Qt::Unchecked;
    }
    if (suspendingObservers)
        suspendObservers();

    job->cmdList << QString("set %1").arg(getTrialCmd());
    if (isStochastic)
    {
        job->cmdList << QString("%1 %2 run_set_multiple %3 %4")
                        .arg(MainWindow::instance()->quietMode)
                        .arg(SessionManager::instance()->currentTrial())
                        .arg(getStochasticCmd())
                        .arg(getStimulusSet());
    }
    else
    {
        job->cmdList << QString("%1 %2 run_set %3")
                        .arg(MainWindow::instance()->quietMode)
                        .arg(SessionManager::instance()->currentTrial())
                        .arg(getStimulusSet());
    }
    job->cmdList << QString("unset %1").arg(getArguments().join(" "));
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

void TrialWidget::updateModelStochasticity(QDomDocument *modelDescription)
{
    QDomElement rootElement = modelDescription->documentElement();
    QDomElement stochasticityElement = XMLAccessor::childElement(rootElement, "stochastic?");
    QString stochasticity = XMLAccessor::value(stochasticityElement);
    if (stochasticity == "1")
        isStochastic = true;
    else
        isStochastic = false;

    setStochasticityVisible(runAllMode && isStochastic);
}

void TrialWidget::observerEnabled(QString name)
{
    if (name.isEmpty())
    {
        LazyNutJob *job = qobject_cast<LazyNutJob *>(sender());
        if (!job)
        {
            qDebug() << "TrialWidget::observerEnabled sender not a LazyNutJob";
            return;
        }
        name = job->data.toMap().value("observer").toString();
        if (name.isEmpty())
        {
            qDebug() << "TrialWidget::observerEnabled LazyNutJob does not contain an observer name";
            return;
        }
    }
    enabledObservers.insert(name);
}

void TrialWidget::observerDisabled(QString name)
{
    if (name.isEmpty())
    {
        LazyNutJob *job = qobject_cast<LazyNutJob *>(sender());
        if (!job)
        {
            qDebug() << "TrialWidget::observerDisabled sender not a LazyNutJob";
            return;
        }
        name = job->data.toMap().value("observer").toString();
        if (name.isEmpty())
        {
            qDebug() << "TrialWidget::observerDisabled LazyNutJob does not contain an observer name";
            return;
        }
    }
    enabledObservers.remove(name);
}

void TrialWidget::suspendObservers()
{
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    foreach(QString observer, enabledObservers)
        job->cmdList << QString("%1 disable").arg(observer);

    SessionManager::instance()->submitJobs(job);
}

void TrialWidget::restoreObservers()
{
    LazyNutJob *job = new LazyNutJob;
    job->logMode |= ECHO_INTERPRETER;
    foreach(QString observer, enabledObservers)
        job->cmdList << QString("%1 enable").arg(observer);

    SessionManager::instance()->submitJobs(job);
}

void TrialWidget::setStochasticityVisible(bool isVisible)
{
    if (isVisible)
    {
        repetitionsLabel->show();
        repetitionsBox->show();
        strategyLabel->show();
        strategyBox->show();
    }
    else
    {
        repetitionsLabel->hide();
        repetitionsBox->hide();
        strategyLabel->hide();
        strategyBox->hide();
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
    setStochasticityVisible(false);

//    clearArgumentBoxes();
    clearDollarArgumentBoxes();
    runAllMode = false;
//    emit runAllModeChanged(false);

}

void TrialWidget::showSetComboBox()
{
    setComboBox->show();
    setCancelButton->show();
    setStochasticityVisible(isStochastic);
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
