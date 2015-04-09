#include "lazynutlistmenu.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"

LazyNutListMenu::LazyNutListMenu(QWidget *parent) :
    getListCmd(""), QMenu(parent)
{
    connect(this, SIGNAL(aboutToShow()), this, SLOT(getList()));
    connect(this, SIGNAL(triggered(QAction*)), this, SLOT(textFromAction(QAction*)));
}

void LazyNutListMenu::setGetListCmd(QString cmd)
{
    getListCmd = cmd;
}

void LazyNutListMenu::getList()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = {getListCmd};
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(this, SLOT(buildMenu(QStringList)));
    SessionManager::instance()->setupJob(param, sender());
}

void LazyNutListMenu::buildMenu(QStringList list)
{
    clear();
    foreach(QString text, list)
        addAction(text);
}

void LazyNutListMenu::textFromAction(QAction *action)
{
    emit selected(action->text());
}
