#include "trialwidget.h"
#include "objectcataloguefilter.h"
#include "descriptionupdater.h"

#include <QComboBox>
#include <QWidget>
#include <QHBoxLayout>
#include <QDomDocument>
#include <QDebug>


TrialWidget::TrialWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout;
    setLayout(layout);

    trialFilter = new ObjectCatalogueFilter(this);
//    trialFilter->setName("");
    trialDescriptionUpdater = new DescriptionUpdater(this);
    trialDescriptionUpdater->setProxyModel(trialFilter);


}

TrialWidget::~TrialWidget()
{
}

void TrialWidget::update(QString trialName)
{
    qDebug() << "Entered trialwidget update";
    trialFilter->setName(trialName);
    qDebug() << "called setName";
}

void TrialWidget::buildComboBoxes(QDomDocument* domDoc)
{
//    layout->addWidget(TCB);


}


