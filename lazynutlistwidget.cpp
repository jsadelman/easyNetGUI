#include "lazynutlistwidget.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"


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
        LazyNutJobParam *param = new LazyNutJobParam;
        param->logMode |= ECHO_INTERPRETER; // debug purpose
        param->cmdList = QStringList({QString("xml %1").arg(getListCmd)});
        param->answerFormatterType = AnswerFormatterType::ListOfValues;
        param->setAnswerReceiver(this, SLOT(buildList(QStringList)));
        SessionManager::instance()->setupJob(param, sender());
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
