#include "trialwidget.h"
#include "objectcataloguefilter.h"
#include "descriptionupdater.h"
#include "xmlelement.h"
#include "mycombobox.h"

#include <QComboBox>
#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>
#include <QDomDocument>
#include <QDebug>
#include <QToolButton>
#include <QAction>
#include <QLineEdit>


TrialWidget::TrialWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout;
    layout1 = new QHBoxLayout;
    layout2 = new QHBoxLayout;
    layout3 = new QVBoxLayout;



    trialFilter = new ObjectCatalogueFilter(this);
//    trialFilter->setName(""); // ADD THIS LINE TO AVOID SEGFAULT ERROR
    trialDescriptionUpdater = new DescriptionUpdater(this);
    trialDescriptionUpdater->setProxyModel(trialFilter);
    connect(trialDescriptionUpdater,SIGNAL (descriptionUpdated(QDomDocument*)),
            this,SLOT(buildComboBoxes(QDomDocument*)));


    runAction = new QAction(QIcon(":/images/media-play-8x.png"),tr("&Run"), this);
    runAction->setStatusTip(tr("Run"));
    connect(runAction,SIGNAL(triggered()),parent,SLOT(runTrial()));
    runButton = new QToolButton(this);
    runButton->hide();

    hideSetComboBoxAction = new QAction(QIcon(":/images/icon_dismiss.png"),tr("&Hide"), this);
    hideSetComboBoxAction->setStatusTip(tr("Hide"));
    connect(hideSetComboBoxAction,SIGNAL(triggered()),this,SLOT(hideSetComboBox()));
}

TrialWidget::~TrialWidget()
{
}

void TrialWidget::update(QString trialName)
{
//    trialDescriptionUpdater->requestDescription(trialName); // 3/7/15 -> added cos currently no descriptions for trials
    qDebug() << "Entered trialwidget update" << trialName;
    if (trialName.isEmpty())
        return;
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
        argumentMap[args[i]]->acceptDrops();
        connect(argumentMap[args[i]], SIGNAL(editTextChanged(QString)),this,SLOT(setRunButtonIcon()));
        connect(argumentMap[args[i]], SIGNAL(argWasChanged(QString)),this,SLOT(argWasChanged(QString)));

        labelList.push_back(new QLabel(args[i]+":"));
        layout2->addWidget(labelList[i]);
        layout2->addWidget(argumentMap[args[i]]);
    }

    QLineEdit *ed = argumentMap[args[args.count()-1]]->lineEdit();
    connect(ed, SIGNAL(returnPressed()),runAction,SIGNAL(triggered()));

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
}

void TrialWidget::argWasChanged(QString arg)
{
    argChanged = arg;
}

void TrialWidget::clearLayout(QLayout *layout)
{
    qDebug() << "Entered clearLayout" << layout;
    QLayoutItem *item;
    while((item = layout->takeAt(0)))
    {
        if (item->layout())
        {
            qDebug() << "found layout" << item->layout();
            clearLayout(item->layout());
//            delete item->layout();
        }
        if (item->widget())
        {
            qDebug() << "found widget" << item->widget();
            delete item->widget();
        }
        qDebug() << "deleting item" << item;
        delete item;
    }
    qDebug() << "Completed clearLayout" << layout;


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
    emit runAllModeChanged(false);

}

void TrialWidget::showSetComboBox()
{
    setComboBox->show();
    setCancelButton->show();
    emit runAllModeChanged(true);
}

void TrialWidget::showSetLabel(QString set)
{
    qDebug() << "Entered showsetLabel";
    showSetComboBox();
    setComboBox->addItem(set);
    setComboBox->setCurrentIndex(setComboBox->findData(set,Qt::DisplayRole));

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
