#include "lazynutlistmenu.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"

LazyNutListMenu::LazyNutListMenu(QWidget *parent) :
    getListCmd(""), QMenu(parent)
{
    connect(this, SIGNAL(aboutToShow()), this, SLOT(getList()));
    connect(this, SIGNAL(triggered(QAction*)), this, SLOT(textFromAction(QAction*)));
}

void LazyNutListMenu::prePopulate(QString text)
{
    preItems.append(text);
}

void LazyNutListMenu::setGetListCmd(QString cmd)
{
    getListCmd = cmd;
}

void LazyNutListMenu::getList()
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({getListCmd});
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(this, SLOT(buildMenu(QStringList)));
    SessionManager::instance()->setupJob(param, sender());
}

void LazyNutListMenu::buildMenu(QStringList list)
{
    clear();
    if (!preItems.empty())
    {
        foreach(QString text, preItems)
            addAction(text);

        addSeparator();
    }
    foreach(QString text, list)
        addAction(text);
}

void LazyNutListMenu::textFromAction(QAction *action)
{
    emit selected(action->text());
}
