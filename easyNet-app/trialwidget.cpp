#include "trialwidget.h"
#include "objectcachefilter.h"
#include "objectupdater.h"
#include "xmlelement.h"
#include "mycombobox.h"
#include "sessionmanager.h"
#include "easyNetMainWindow.h"
#include "lazynutjob.h"
#include "plotviewer.h"
#include "xmlaccessor.h"
#include "dataframeviewer.h"
#include "enumclasses.h"

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
    : trialRunMode(TrialRunMode_Single), askDisableObserver(true), suspendingObservers(false), QWidget(parent)
{
    layout = new QHBoxLayout;
    layout1 = new QHBoxLayout;
    layout2 = new QHBoxLayout;
    layout3 = new QVBoxLayout;

    trialFilter = new ObjectCacheFilter(SessionManager::instance()->descriptionCache, this);
    connect(SessionManager::instance(), SIGNAL(currentTrialChanged(QString)), trialFilter, SLOT(setName(QString)));
    connect(SessionManager::instance(), &SessionManager::currentTrialChanged, [=] (QString name)
    {
        if (name.isEmpty())
            buildComboBoxesTest(); // a hack to clear the layout when all trials are gone, e.g. restart lazynut
    });

//    connect(trialFilter, SIGNAL(objectDestroyed(QString)), this, SLOT(buildComboBoxesTest()));
//    connect(trialFilter, &ObjectCacheFilter::objectDestroyed, [=](QString name)
//    {
//        qDebug() << Q_FUNC_INFO << name << trialFilter;
//    });

    // cosmetics used in tabs names in TableWindow, will be taken care of in trial scripts
    connect(trialFilter, &ObjectCacheFilter::objectCreated, [=](QString name, QString, QString, QDomDocument*)
    {
        QString df = QString("(%1 default_observer)").arg(name);
        SessionManager::instance()->setPrettyName(df, name);
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

    modelDescriptionUpdater = new ObjectUpdater(this);
    modelDescriptionUpdater->setProxyModel(modelFilter);
    connect(modelDescriptionUpdater,SIGNAL(objectUpdated(QDomDocument*, QString)),
           this, SLOT(updateModelStochasticity(QDomDocument*)));


    runAction = new QAction(tr("&Run"), this);
    runAction->setStatusTip(tr("Run"));
    connect(runAction,SIGNAL(triggered()),this,SLOT(runTrial()));
    runButton = new QToolButton(this);
    runButton->hide();

    hideSetComboBoxAction = new QAction(QIcon(":/images/icon_dismiss.png"),tr("&Hide"), this);
    hideSetComboBoxAction->setStatusTip(tr("Hide"));
    connect(hideSetComboBoxAction,SIGNAL(triggered()),this,SLOT(hideSetComboBox()));

    buildComboBoxesTest();

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
    QStringList argList;
//    defs.clear();
    XMLelement arg = XMLelement(*domDoc)["arguments"].firstChild();
    while (!arg.isNull())
    {
        argList.append(arg.label());
        defs.insert(arg.label(), defs.keys().contains(arg.label()) && (arg.value().isNull() || arg.value() == "NULL") ?
                    defs[arg.label()] : arg.value());
//        defs[arg.label()]=(arg.value());
        arg = arg.nextSibling();
    }
    foreach(QString label, defs.keys().toSet().subtract(argList.toSet()))
        defs.remove(label);

    if (argList.size())
        buildComboBoxesTest(argList);

}

void TrialWidget::buildComboBoxesTest(QStringList args)
{
    // save defs
    foreach(QString label, argumentMap.keys())
        defs[label] = argumentMap[label]->currentText();
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
        runTrialList(job);
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
}

QString TrialWidget::defaultDataframe()
{
    return QString("(%1 default_observer)")
            .arg(SessionManager::instance()->currentTrial());
}

void TrialWidget::runSingleTrial(LazyNutJob *job)
{
    job->cmdList << QString("%1 %2 step %3")
            .arg(MainWindow::instance()->quietMode)
            .arg(SessionManager::instance()->currentTrial())
            .arg(getTrialCmd());
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

    QDomDocument * stimulusSetDescription = SessionManager::instance()->descriptionCache->getDomDoc(SessionManager::instance()->currentSet());
    int trialListLength = stimulusSetDescription ? XMLelement(*stimulusSetDescription)["rows"]().toInt() : 1;
    if (isStochastic)
        trialListLength *= repetitionsBox->currentText().isEmpty() ? 1 : repetitionsBox->currentText().toInt();
    MainWindow::instance()->setTrialListLength(trialListLength);
    MainWindow::instance()->updateTrialRunListCount(0);
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

    setStochasticityVisible(trialRunMode == TrialRunMode_List && isStochastic);
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
    clearDollarArgumentBoxes();
    trialRunMode = TrialRunMode_Single;
    emit trialRunModeChanged(trialRunMode);

}

void TrialWidget::showSetComboBox()
{
    setComboBox->show();
    setCancelButton->show();
    setStochasticityVisible(isStochastic);
    trialRunMode = TrialRunMode_List;
    emit trialRunModeChanged(trialRunMode);
}

void TrialWidget::showSetLabel(QString set)
{
    showSetComboBox();
    setComboBox->addItem(set);
    setComboBox->setCurrentIndex(setComboBox->findData(set,Qt::DisplayRole));
    SessionManager::instance()->setCurrentSet(set);
}

void TrialWidget::restoreComboBoxText()
{
    if (argChanged.isEmpty())
        return;
    if (argumentMap.isEmpty())
        return;
    myComboBox* box = static_cast<myComboBox*>(argumentMap[argChanged]);
    box->restoreComboBoxText();

}

QString TrialWidget::getStimulusSet()
{
    return setComboBox->currentText();
}
