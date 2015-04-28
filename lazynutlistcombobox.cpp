#include "lazynutlistcombobox.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"

LazyNutListComboBox::LazyNutListComboBox(QString getListCmd, QWidget *parent) :
    getListCmd(getListCmd), QComboBox(parent)
{
    setEditable(false);

}

void LazyNutListComboBox::setEmptyItem(bool present)
{
    if (present && !itemText(0).isEmpty())
        insertItem(0, "");

    else if (!present && itemText(0).isEmpty())
        removeItem(0);
}

void LazyNutListComboBox::setGetListCmd(QString cmd)
{
    getListCmd = cmd;
}

void LazyNutListComboBox::getList(QString cmd)
{
//    buildList(QStringList({"badger", "mushroom", "snake"}));

    getListCmd = cmd.isEmpty() ? getListCmd : cmd;
    if (!getListCmd.isEmpty())
    {
        LazyNutJobParam *param = new LazyNutJobParam;
//        param->logMode |= ECHO_INTERPRETER; // debug purpose
        param->cmdList = QStringList({QString("xml %1").arg(getListCmd)});
        param->answerFormatterType = AnswerFormatterType::ListOfValues;
        param->setAnswerReceiver(this, SLOT(buildList(QStringList)));
        SessionManager::instance()->setupJob(param, sender());
    }
    else
    {
        clear();
        emit listReady();
    }
}

void LazyNutListComboBox::buildList(QStringList list)
{
    clear();
    addItems(list);
    emit listReady();
}
