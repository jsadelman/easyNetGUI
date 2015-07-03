#include "trialwidget.h"
#include "objectcataloguefilter.h"
#include "descriptionupdater.h"
#include "xmlelement.h"

#include <QComboBox>
#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>
#include <QDomDocument>
#include <QDebug>
#include <QToolButton>
#include <QAction>


TrialWidget::TrialWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout;
    setLayout(layout);

    trialFilter = new ObjectCatalogueFilter(this);
    trialFilter->setName(""); // ADD THIS LINE TO AVOID SEGFAULT ERROR
    trialDescriptionUpdater = new DescriptionUpdater(this);
    trialDescriptionUpdater->setProxyModel(trialFilter);
    connect(trialDescriptionUpdater,SIGNAL (descriptionUpdated(QDomDocument*)),
            this,SLOT(buildComboBoxes(QDomDocument*)));


    runAction = new QAction(QIcon(":/images/media-play-8x.png"),tr("&Run"), this);
    runAction->setStatusTip(tr("Run"));
    connect(runAction,SIGNAL(triggered()),parent,SLOT(runTrial()));
    runButton = new QToolButton(this);
    runButton->hide();


}

TrialWidget::~TrialWidget()
{
}

void TrialWidget::update(QString trialName)
{
    trialDescriptionUpdater->requestDescription(trialName); // 3/7/15 -> added cos currently no descriptions for trials
    qDebug() << "Entered trialwidget update";
    trialFilter->setName(trialName);
    qDebug() << "called setName";

}

void TrialWidget::buildComboBoxes(QDomDocument* domDoc)
{
    QStringList argList;
    XMLelement arg = XMLelement(*domDoc)["arguments"].firstChild();
    while (!arg.isNull())
    {
        argList.append(arg.label());
        arg = arg.nextSibling();
    }
    qDebug() << "buildComboBoxes args = " << argList;
    if (argList.size())
        buildComboBoxesTest(argList);

}

void TrialWidget::buildComboBoxesTest(QStringList args)
{
    qDebug() << "Entered buildComboBoxesTest";
    // first delete existing labels/boxes/button
    QMap<QString, QComboBox*>::const_iterator i = argumentMap.constBegin();
    while (i != argumentMap.constEnd())
    {
        delete i.value();
        ++i;
    }
    argumentMap.clear();

    for (int i=0;i<labelList.count();i++)
        delete labelList[i];
    labelList.clear();
    delete runButton;

    // now add new boxes
    for (int i=0;i<args.count();i++)
    {
        qDebug() << "Adding" << args[i];
        argumentMap[args[i]] = new QComboBox(this);
        argumentMap[args[i]]->setEditable(true);
        argumentMap[args[i]]->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        argumentMap[args[i]]->setMinimumSize(100, argumentMap[args[i]]->minimumHeight());
        connect(argumentMap[args[i]], SIGNAL(editTextChanged(QString)),this,SLOT(setRunButtonIcon()));
        labelList.push_back(new QLabel(args[i]+":"));
        layout->addWidget(labelList[i]);
        layout->addWidget(argumentMap[args[i]]);
    }

    runButton = new QToolButton(this);
    runButton->setAutoRaise(true);
    runButton->setDefaultAction(runAction);
    runButton->setIcon(QIcon(":/images/run_disabled.png"));
    runButton->setIconSize(QSize(28, 28)); // (QSize(32, 32));
    runButton->show();
    layout->addWidget(runButton);

}

QString TrialWidget::getTrialCmd()
{
    QString cmd;
    QMap<QString, QComboBox*>::const_iterator i = argumentMap.constBegin();
    while (i != argumentMap.constEnd())
    {
        cmd += " ";
        cmd += i.key();
        cmd += "=";
        cmd += static_cast<QComboBox*>(argumentMap[i.key()])->currentText();
        ++i;
    }
    return (cmd);


}

bool TrialWidget::checkIfReadyToRun()
{
    QMap<QString, QComboBox*>::const_iterator i = argumentMap.constBegin();
    while (i != argumentMap.constEnd())
    {
        if (static_cast<QComboBox*>(argumentMap[i.key()])->currentText().isEmpty())
            return false;
        i++;
    }
    return true;
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


