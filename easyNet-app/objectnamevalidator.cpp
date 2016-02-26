#include "objectnamevalidator.h"
#include "objectcache.h"
#include "sessionmanager.h"
#include "enumclasses.h"

ObjectNameValidator::ObjectNameValidator(QObject *parent)
    : QValidator(parent)
{
    QStringList forbiddenNames = QStringList()
                      << "\\s+.*"
                      << "[#(0-9].*"
                      << "query"
                      << "xml"
                      << "xmllint"
                      << "quietly"
                      << "loglevel"
                      << "CRASH"
                      << "recently_created"
                      << "clear_recently_created"
                      << "recently_destroyed"
                      << "clear_recently_destroyed"
                      << "recently_modified"
                      << "clear_recently_modified"
                      << "version"
                      << "shush"
                      << "unshush"
                      << "include"
                      << "create"
                      << "destroy"
                      << "load"
                      << "until"
                      << "if"
                      << "list"
                      << "facets"
                      << "loop"
                      << "less"
                      << "or"
                      << "and"
                      << "default_model"
                      << "set_default_model"
                      << "named_loop"
                      << "creators"
                      << "aesthetic"
                      << "stop"
                      << "watchlist"
                      << "limit_descriptions"
                      << "unlimit_descriptions"
                      << "set"
                      << "get"
                      << "unset"
                      << "R"
                         ;
    forbiddenRex = QRegExp(QString("^(%1)$").arg(forbiddenNames.join("|")));
}

QValidator::State ObjectNameValidator::validate(QString &input, int &pos) const
{
    Q_UNUSED(pos)
    if (forbiddenRex.exactMatch(input) || SessionManager::instance()->exists(input) ||
            SessionManager::instance()->extraNamedItems.contains(input))
        return Invalid;

    else
        return Acceptable;

}

bool ObjectNameValidator::isValid(QString name)
{
    int pos(0);
    return validate(name, pos) == QValidator::Acceptable;
}

QString ObjectNameValidator::makeValid(QString name)
{
    // first eliminate brackets and spaces
    name = normalisedName(name);
    QRegExp startsWithDigitRex("^\\d+");
    if (startsWithDigitRex.indexIn(name) >= 0)
        name.prepend("_");
    // then, if name is not valid appends .1 or .2 etc. until a valid name is found.
    if (isValid(name))
        return name;

    QRegExp countRx("\\.(\\d+)$");
    int count = 1;
    if (countRx.indexIn(name) == -1)
    {
        name.append(".1");
    }
    else
    {
        count = countRx.cap(1).toInt();
    }
    while (!isValid(name))
    {
        name.replace(QRegExp(QString("\\.%1$").arg(QString::number(count))),
                     QString(".%1").arg(QString::number(count + 1)));
        ++count;
    }
    return name;
}
