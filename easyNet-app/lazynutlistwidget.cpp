#include "lazynutlistwidget.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"
#include "lazynutjob.h"


LazyNutListWidget::LazyNutListWidget(QString getListCmd, QWidget *parent) :
    getListCmd(getListCmd), QListWidget(parent)
{
    if (!getListCmd.isEmpty())
        getList();
    connect(this, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(item2text(QListWidgetItem*)));
}

void LazyNutListWidget::setGetListCmd(QString cmd)
{
    getListCmd = cmd;
}

void LazyNutListWidget::getList(QString cmd)
{
    getListCmd = cmd.isEmpty() ? getListCmd : cmd;
    if (!getListCmd.isEmpty())
    {
        LazyNutJob *job = new LazyNutJob;
        job->cmdList = QStringList({QString("xml %1").arg(getListCmd)});
        job->setAnswerReceiver(this, SLOT(buildList(QStringList)), AnswerFormatterType::ListOfValues);
        SessionManager::instance()->submitJobs(job);
    }
}

QString LazyNutListWidget::currentValue()
{
    return currentItem()->text();
}

void LazyNutListWidget::setCurrentValue(QString value)
{
    QList<QListWidgetItem*> selectedItem = findItems(value, Qt::MatchFixedString|Qt::MatchCaseSensitive);
    if (!selectedItem.isEmpty())
        setCurrentItem(selectedItem.at(0));
}

void LazyNutListWidget::buildList(QStringList list)
{
    clear();
    addItems(list);
}

void LazyNutListWidget::item2text(QListWidgetItem *item)
{
    emit selected(item->text());
}
