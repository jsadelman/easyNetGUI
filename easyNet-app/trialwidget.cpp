#include "trialwidget.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "xmlelement.h"
#include "sessionmanager.h"
#include "easyNetMainWindow.h"
#include "lazynutjob.h"
#include "plotviewer.h"
#include "xmlaccessor.h"
#include "dataframeviewer.h"
#include "enumclasses.h"
#include "dataframemodel.h"
#include "droplineedit.h"

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

Q_DECLARE_METATYPE(QSharedPointer<QDomDocument>)


TrialWidget::TrialWidget(QWidget *parent)
    : trialRunMode(TrialRunMode_Single), askDisableObserver(true),
      suspendingObservers(false), QWidget(parent), currentParamExplore(""),
      isStochastic(false)
{
    trialFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    connect(SessionManager::instance(), SIGNAL(currentTrialChanged(QString)), trialFilter, SLOT(setName(QString)));
    connect(SessionManager::instance(), &SessionManager::currentTrialChanged, [=] (QString name)
    {
        if (name.isEmpty())
        {
            execBuildComboBoxes();
        }
    });

    trialDescriptionUpdater = new ObjectUpdater(this);
    trialDescriptionUpdater->setProxyModel(trialFilter);
    connect(trialDescriptionUpdater,SIGNAL (objectUpdated(QDomDocument*, QString)),
            this,SLOT(buildComboBoxes(QDomDocument*)));
//    connect(trialDescriptionUpdater,SIGNAL (objectUpdated(QDomDocument*, QString)),
//            this,SIGNAL(trialDescriptionUpdated(QDomDocument*)));

    modelFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    connect(SessionManager::instance(), SIGNAL(currentModelChanged(QString)),
            modelFilter, SLOT(setName(QString)));
    connect(SessionManager::instance(), &SessionManager::currentModelChanged, [=](QString name)
    {
        if (name.isEmpty())
        {
            setTrialRunMode(TrialRunMode_Single);
            setStochasticityVisible(false);
        }
    });

    modelDescriptionUpdater = new ObjectUpdater(this);
    modelDescriptionUpdater->setProxyModel(modelFilter);
    connect(modelDescriptionUpdater,SIGNAL(objectUpdated(QDomDocument*, QString)),
           this, SLOT(updateModelStochasticity(QDomDocument*)));


    paramExploreFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    paramExploreDescriptionUpdater = new ObjectUpdater(this);
    paramExploreDescriptionUpdater->setProxyModel(paramExploreFilter);

    paramExploreDataframeFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);
    paramExploreDataframeUpdater = new ObjectUpdater(this);
    paramExploreDataframeUpdater->setProxyModel(paramExploreDataframeFilter);
    connect(paramExploreDataframeUpdater, SIGNAL(objectUpdated(QDomDocument*,QString)),
            this, SLOT(runParamExplore(QDomDocument*,QString)));

    setFilter = new ObjectCacheFilter(SessionManager::instance()->dataframeCache, this);

    hideSetComboBoxAction = new QAction(QIcon(":/images/icon_dismiss.png"),tr("&Hide"), this);
    hideSetComboBoxAction->setStatusTip(tr("Hide"));
//    connect(hideSetComboBoxAction,SIGNAL(triggered()),this,SLOT(hideSetComboBox()));
    connect(hideSetComboBoxAction, &QAction::triggered, [=](){setTrialRunMode(TrialRunMode_Single);});

    buildWidget();
    setTrialRunMode(TrialRunMode_Single);
//    hideSetComboBox();

    disableObserversMsg = new QMessageBox(
                QMessageBox::Question,
                "Suspend layer activity recording",
                "You are about to run a list of trials while layer activity is being recorded and displayed in plots. "
                "This may slow down the simulation.\n"
                "Do you want to suspend activity recording while running a list of trials?",
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                this);
    dontAskAgainDisableObserverCheckBox = new QCheckBox("don't show this message again");
    disableObserversMsg->setCheckBox(dontAskAgainDisableObserverCheckBox);
}

TrialWidget::~TrialWidget()
{
}

void TrialWidget::update(QString trialName)
{
    if (trialName.isEmpty())
        return;
    trialFilter->setName(trialName);
}

void TrialWidget::buildComboBoxes(QDomDocument* domDoc)
{
    QStringList args;
    XMLelement arg = XMLelement(*domDoc)["arguments"].firstChild();
    while (!arg.isNull())
    {
        args.append(arg.label());
        defs.insert(arg.label(), defs.keys().contains(arg.label()) && (arg.value().isNull() || arg.value() == "NULL") ?
                    defs[arg.label()] : arg.value());
//        defs[arg.label()]=(arg.value());
        arg = arg.nextSibling();
    }
    foreach(QString label, defs.keys().toSet().subtract(args.toSet()))
        defs.remove(label);

    if (args.size())
        execBuildComboBoxes(args);
}

void TrialWidget::execBuildComboBoxes(QStringList args)
{
    // clear layout2
    foreach (QString arg, argList)
    {
        layout2->removeWidget(labelMap.value(arg));
        labelMap.value(arg)->setVisible(false);
        layout2->removeWidget(comboMap.value(arg));
        comboMap.value(arg)->setVisible(false);
    }
    argList = args;

    // build new widgets if needed
    foreach (QString arg, args)
    {
        if (!labelMap.contains(arg))
        {
            labelMap[arg] = new QLabel(QString("%1:").arg(arg), this);
        }
        else
        {
            labelMap.value(arg)->setVisible(true);
        }
        if (!comboMap.contains(arg))
        {
            comboMap[arg] = new QComboBox(this);

            comboMap[arg]->setEditable(true);
            comboMap[arg]->setLineEdit(new DropLineEdit(comboMap[arg]));
            comboMap[arg]->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            comboMap[arg]->setMinimumSize(100, comboMap[arg]->minimumHeight());
            connect(comboMap[arg], SIGNAL(editTextChanged(QString)),this,SLOT(setRunButtonIcon()));
        }
        else
        {
            comboMap.value(arg)->setVisible(true);
        }
    }
    // repopulate layout2
    foreach (QString arg, args)
    {
        layout2->addWidget(labelMap.value(arg));
        layout2->addWidget(comboMap.value(arg));
    }
    // set defaults
    foreach (QString arg, defs.keys())
    {
        if (comboMap.contains(arg) && comboMap.value(arg)->currentText().isEmpty())
            comboMap.value(arg)->setCurrentText(defs.value(arg));
    }

    if (!args.isEmpty())
    {
        foreach (QString arg, args)
        {
            QLineEdit *ed = comboMap.value(arg)->lineEdit();
            disconnect(ed, SIGNAL(returnPressed()),runAction,SIGNAL(triggered()));
        }
        QLineEdit *ed = comboMap.value(args.last())->lineEdit();
        connect(ed, SIGNAL(returnPressed()),runAction,SIGNAL(triggered()));
    }

    if (args.isEmpty())
        runButton->hide();
    else
    {
        runButton->show();
        setRunButtonIcon();
        if (!hasDollarArguments())
            setTrialRunMode(TrialRunMode_Single);
    }
}


void TrialWidget::clearLayout(QLayout *layout)
{
    if (!layout)
        return;
    QLayoutItem *item;
    while((item = layout->takeAt(0)))
    {
        if (item->layout())
        {
            clearLayout(item->layout());
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
    foreach(QString arg, argList)
    {
        cmd.append(QString(" %1=%2").arg(arg).arg(comboMap.value(arg)->currentText()));
    }
    return (cmd);
}

QString TrialWidget::getStochasticCmd()
{
    switch (trialRunMode)
    {
    case TrialRunMode_Single:
        return repetitionsBox->currentText().isEmpty() ? "1" : repetitionsBox->currentText();
    case TrialRunMode_List:
        return QString("%1 %2")
            .arg(strategyBox->currentText())
            .arg(repetitionsBox->currentText().isEmpty() ? "1" : repetitionsBox->currentText());
    default:
        return QString();
    }
}

QStringList TrialWidget::getArguments()
{
    return(comboMap.keys());
}

void TrialWidget::runTrial()
{
    if (SessionManager::instance()->currentModel().isEmpty())
    {
        QMessageBox::warning(this, "Help", "Choose which model to run");
        return;
    }
    if (SessionManager::instance()->currentTrial().isEmpty())
    {
        QMessageBox::warning(this, "Help", "Choose which type of trial to run");
        return;
    }
    if (trialRunMode == TrialRunMode_List && askDisableObserver && !SessionManager::instance()->enabledObservers().isEmpty())
    {
        int answer = disableObserversMsg->exec();
        if (answer == QMessageBox::Cancel)
            return;
        SessionManager::instance()->suspendObservers(answer == QMessageBox::Yes);
        askDisableObserver = dontAskAgainDisableObserverCheckBox->checkState() == Qt::Unchecked;
    }
    QSharedPointer<QDomDocument> trialRunInfo = createTrialRunInfo();
    emit aboutToRunTrial(trialRunInfo);

    LazyNutJob *job = new LazyNutJob;
    if (trialRunMode != TrialRunMode_List || !SessionManager::instance()->suspendingObservers())
    {
        foreach(QString observer, SessionManager::instance()->enabledObservers())
            job->cmdList << QString("%1 clear").arg(observer);
    }
    if (trialRunMode == TrialRunMode_List)
    {
        runTrialList(job);
        QDomDocument * stimulusSetDescription = SessionManager::instance()->descriptionCache->getDomDoc(SessionManager::instance()->currentSet());
        int trialListLength = stimulusSetDescription ? XMLelement(*stimulusSetDescription)["rows"]().toInt() : 1;
        if (isStochastic)
            trialListLength *= repetitionsBox->currentText().isEmpty() ? 1 : repetitionsBox->currentText().toInt();
        MainWindow::instance()->setTrialListLength(trialListLength);
        MainWindow::instance()->updateTrialRunListCount(0);
    }
    else
        runSingleTrial(job);

    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->updateObjectCacheJobs();

    QMap<QString, QVariant> jobData;
    jobData.insert("trialRunInfo", QVariant::fromValue(trialRunInfo));
    jobs.last()->data = jobData;
    jobs.last()->appendEndOfJobReceiver(MainWindow::instance()->dataframeResultsViewer, SLOT(dispatch()));
    jobs.last()->appendEndOfJobReceiver(MainWindow::instance()->plotViewer, SLOT(dispatch()));

    if (trialRunMode == TrialRunMode_List)
    {
        MainWindow::instance()->runAllTrialMsgAct->setVisible(true);
        job->appendEndOfJobReceiver(MainWindow::instance(), SIGNAL(runAllTrialEnded()));
    }
    SessionManager::instance()->submitJobs(jobs);
    insertArgumentsInBoxes();
}

QString TrialWidget::defaultDataframe()
{
    return QString("(%1 default_observer)")
            .arg(SessionManager::instance()->currentTrial());
}

void TrialWidget::runSingleTrial(LazyNutJob *job)
{
    if (isStochastic)
    {
        job->cmdList << QString("%1 %2 multistep %3 %4")
                        .arg(MainWindow::instance()->quietMode)
                        .arg(SessionManager::instance()->currentTrial())
                        .arg(getStochasticCmd())
                        .arg(getTrialCmd());
    }
    else
    {
        job->cmdList << QString("%1 %2 step %3")
                        .arg(MainWindow::instance()->quietMode)
                        .arg(SessionManager::instance()->currentTrial())
                        .arg(getTrialCmd());
    }
}

QSharedPointer<QDomDocument> TrialWidget::createTrialRunInfo()
{
    QSharedPointer<QDomDocument> trialRunInfo (new QDomDocument);
    QDomElement rootElem = trialRunInfo->createElement("string");
    rootElem.setAttribute("label", "Title");
    rootElem.setAttribute("value", "Trial run info");
    trialRunInfo->appendChild(rootElem);
    QDomElement trialElem = trialRunInfo->createElement("string");
    trialElem.setAttribute("label", "Trial");
    trialElem.setAttribute("value", SessionManager::instance()->currentTrial());
    rootElem.appendChild(trialElem);
    QDomElement modeElem = trialRunInfo->createElement("string");
    modeElem.setAttribute("label", "Run mode");
    modeElem.setAttribute("value", trialRunModeName.value(trialRunMode));
    rootElem.appendChild(modeElem);
    QDomElement dataframeElem = trialRunInfo->createElement("object");
    dataframeElem.setAttribute("label", "Results");
    dataframeElem.setAttribute("value", QString("(%1 default_observer)")
                               .arg(SessionManager::instance()->currentTrial()
                               ));
    rootElem.appendChild(dataframeElem);
    QDomElement valuesElem = trialRunInfo->createElement("map");
    valuesElem.setAttribute("label", "Values");
    QMapIterator<QString, QComboBox*> argumentMap_it(comboMap);
    while(argumentMap_it.hasNext())
    {
        argumentMap_it.next();
        QDomElement valueElem = trialRunInfo->createElement("string");
        valueElem.setAttribute("label", argumentMap_it.key());
        valueElem.setAttribute("value", comboMap[argumentMap_it.key()]->currentText());
        valuesElem.appendChild(valueElem);
    }
    rootElem.appendChild(valuesElem);

    return trialRunInfo;
}

void TrialWidget::runTrialList(LazyNutJob *job)
{
    if (SessionManager::instance()->suspendingObservers())
        foreach(QString observer, SessionManager::instance()->enabledObservers())
            job->cmdList << QString("%1 disable").arg(observer);

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
    if (SessionManager::instance()->suspendingObservers())
        foreach(QString observer, SessionManager::instance()->enabledObservers())
            job->cmdList << QString("%1 enable").arg(observer);


}

bool TrialWidget::checkIfReadyToRun()
{
    foreach(QString arg, argList)
    {
        if (comboMap.value(arg)->currentText().isEmpty())
            return false;
    }
    return true;
}

void TrialWidget::clearArgumentBoxes()
{
    QMap<QString, QComboBox*>::const_iterator i = comboMap.constBegin();

    while (i != comboMap.constEnd())
    {
        comboMap[i.key()]->clearEditText();
        comboMap[i.key()]->setCurrentText(defs[i.key()]);
        i++;
    }
}

void TrialWidget::clearDollarArgumentBoxes()
{
    QMap<QString, QComboBox*>::const_iterator i = comboMap.constBegin();
    while (i != comboMap.constEnd())
    {
        if (comboMap[i.key()]->currentText().startsWith('$'))
        {
            comboMap[i.key()]->clearEditText();
        }
        i++;
    }
}

void TrialWidget::insertArgumentsInBoxes()
{
    QMap<QString, QComboBox*>::const_iterator i = comboMap.constBegin();
    while (i != comboMap.constEnd())
    {
        QString argument = comboMap[i.key()]->currentText();
        if (!argument.isEmpty() && !argument.startsWith('$') && comboMap[i.key()]->findText(argument) < 0)
            comboMap[i.key()]->insertItem(0, argument);

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

    setStochasticityVisible(isStochastic);
}

void TrialWidget::addParamExploreDf(QString name)
{
    paramExploreFilter->addName(name);
    paramExploreDataframeFilter->addName(name);
}

void TrialWidget::initParamExplore(QString name)
{
    if (!paramExploreFilter->contains(name))
        return;
    if (!checkIfReadyToRun())
    {
        QMessageBox::warning(this, "Missing arguments",
                             "The current trial is not completely specified.\n"
                             "Please fill in all arguments in the Trial box on top of the main window and then press the Apply button again."
                             );
        return;
    }
    currentParamExplore = name;
}

void TrialWidget::runParamExplore(QDomDocument *df, QString name)
{
    if (name != currentParamExplore)
        return;

    DataFrameModel dfModel(df, this);
    if (dfModel.rowCount() == 0 || !(dfModel.colNames().contains("parameter") && dfModel.colNames().contains("value")))
        return;

    currentParamExplore = "";
    // compute number of items
    int trialListLength = dfModel.rowCount();
    if (trialRunMode == TrialRunMode_List)
    {
        QDomDocument *description = SessionManager::instance()->description(getStimulusSet());
        if (description)
            trialListLength *= XMLelement(*description)["rows"]().toInt();
    }
    if (isStochastic)
        trialListLength *= repetitionsBox->currentText().isEmpty() ? 1 : repetitionsBox->currentText().toInt();

    MainWindow::instance()->setTrialListLength(trialListLength);
    MainWindow::instance()->updateTrialRunListCount(0);

    SessionManager::instance()->suspendObservers();
    int saved_trialRunMode = trialRunMode;
    trialRunMode = TrialRunMode_List;
    QSharedPointer<QDomDocument> trialRunInfo = createTrialRunInfo();
    emit aboutToRunTrial(trialRunInfo);
    trialRunMode = saved_trialRunMode;

    LazyNutJob *job = new LazyNutJob;
    for (int row = 0; row < dfModel.rowCount(); ++row)
    {
        job->cmdList << QString("(%1 parameters) set %2 %3")
                        .arg(SessionManager::instance()->currentModel())
                        .arg(dfModel.data(dfModel.index(row, 0)).toString())
                        .arg(dfModel.data(dfModel.index(row, 1)).toString());
        if (trialRunMode == TrialRunMode_List)
            runTrialList(job);
        else
            runSingleTrial(job);
    }
    QList<LazyNutJob*> jobs = QList<LazyNutJob*>()
            << job
            << SessionManager::instance()->updateObjectCacheJobs();

    QMap<QString, QVariant> jobData;
    jobData.insert("trialRunInfo", QVariant::fromValue(trialRunInfo));
    jobData.insert("hide", name);
    jobs.last()->data = jobData;
    jobs.last()->appendEndOfJobReceiver(MainWindow::instance()->dataframeResultsViewer, SLOT(dispatch()));
    jobs.last()->appendEndOfJobReceiver(MainWindow::instance()->plotViewer, SLOT(dispatch()));
    jobs.last()->appendEndOfJobReceiver(MainWindow::instance(), SLOT(hideItemFromResults()));
    jobs.last()->appendEndOfJobReceiver(SessionManager::instance(), SLOT(resumeObservers()));

    MainWindow::instance()->runAllTrialMsgAct->setVisible(true);
    job->appendEndOfJobReceiver(MainWindow::instance(), SIGNAL(runAllTrialEnded()));
    SessionManager::instance()->submitJobs(jobs);
}

void TrialWidget::setTrialRunMode(int mode)
{
    switch (mode) {
    case TrialRunMode_Single:
        clearDollarArgumentBoxes();
        trialRunMode = TrialRunMode_Single;
        hideSetComboBox();
        emit trialRunModeChanged(trialRunMode);
        break;
    case TrialRunMode_List:
        trialRunMode = TrialRunMode_List;
        showSetComboBox();
        emit trialRunModeChanged(trialRunMode);
        break;
    default:
        break;
    }

}

void TrialWidget::buildWidget()
{
    setComboBox = new QComboBox(this);
    setComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    setComboBox->setModel(setFilter);
    setCancelButton = new QToolButton(this);
    setCancelButton->setIcon(QIcon(":/images/icon_dismiss.png"));
    setCancelButton->setAutoRaise(true);
    setCancelButton->setDefaultAction(hideSetComboBoxAction);

    // Widget for stochastic models
    repetitionsLabel = new QLabel("Repetitions");
    repetitionsBox = new QComboBox(this);
    repetitionsBox->setEditable(true);
    repetitionsBox->setValidator(new QIntValidator(this));
    repetitionsBox->addItem("1");
    strategyLabel = new QLabel("Strategy");
    strategyBox = new QComboBox(this);
    strategyBox->addItems(QStringList({"ABAB", "AABB"}));
    strategyBox->setEditable(false);

    layout1 = new QHBoxLayout;
    layout1->addWidget(repetitionsLabel);
    layout1->addWidget(repetitionsBox);
    layout1->addWidget(strategyLabel);
    layout1->addWidget(strategyBox);

    layout1->addWidget(setComboBox);
    layout1->addWidget(setCancelButton);

    runAction = new QAction(tr("&Run"), this);
    runAction->setStatusTip(tr("Run"));
    connect(runAction,SIGNAL(triggered()),this,SLOT(runTrial()));
    runButton = new QToolButton(this);
    runButton->setAutoRaise(true);
    runButton->setDefaultAction(runAction);
    runButton->setIcon(QIcon(":/images/run_disabled.png"));
    runButton->setIconSize(QSize(40, 40));
    runButton->hide();

    layout2 = new QHBoxLayout; // used in execBuildComboBoxes
    layout3 = new QVBoxLayout;
    layout3->addLayout(layout1);
    layout3->addLayout(layout2);
    layout = new QHBoxLayout;
    layout->addLayout(layout3);
    layout->addWidget(runButton);
    setLayout(layout);

    setStochasticityVisible(false);

}

void TrialWidget::setStochasticityVisible(bool isVisible)
{
    if (isVisible)
    {
        repetitionsLabel->show();
        repetitionsBox->show();
        if (trialRunMode == TrialRunMode_List)
        {
            strategyLabel->show();
            strategyBox->show();
        }
    }
    else
    {
        repetitionsLabel->hide();
        repetitionsBox->hide();
        strategyLabel->hide();
        strategyBox->hide();
    }
}

bool TrialWidget::hasDollarArguments()
{
    foreach(QString arg, argList)
    {
        if (comboMap.value(arg)->currentText().startsWith('$'))
            return true;
    }
    return false;
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
    if (isStochastic)
    {
        strategyLabel->hide();
        strategyBox->hide();
    }
}

void TrialWidget::showSetComboBox()
{
    setComboBox->show();
    setCancelButton->show();
    if (isStochastic)
    {
        strategyLabel->show();
        strategyBox->show();
    }
}

void TrialWidget::showSetLabel(QString set)
{
    setTrialRunMode(TrialRunMode_List);
    if (setComboBox->findText(set) < 0)
        setFilter->addName(set);
//        setComboBox->addItem(set);

    setComboBox->setCurrentText(set);
    SessionManager::instance()->setCurrentSet(set);
}


QString TrialWidget::getStimulusSet()
{
    return setComboBox->currentText();
}
