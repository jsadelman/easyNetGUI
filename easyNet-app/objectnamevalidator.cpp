#include "objectnamevalidator.h"
#include "objectcache.h"
#include "sessionmanager.h"
#include "enumclasses.h"

ObjectNameValidator::ObjectNameValidator(QObject *parent, QStringList lazyNutkeywords)
    : QValidator(parent)
{
    QStringList forbiddenNames = lazyNutkeywords
                      << ""
                      << "\\s+.*"
                      << "[#(0-9].*"
                      << ".*::.*";
    forbiddenRex = QRegExp(QString("^(%1)$").arg(forbiddenNames.join("|")));
}

QValidator::State ObjectNameValidator::validate(QString &input, int &pos) const
{
    Q_UNUSED(pos)
    if (forbiddenRex.exactMatch(input) || SessionManager::instance()->exists(input) ||
            SessionManager::instance()->m_extraNamedItems.contains(input) ||
            SessionManager::instance()->m_requestedNames.contains(input))
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
    qDebug()<<"will make valid "<<name;
    // first eliminate brackets and spaces
    name = normalisedName(name);

    QRegExp startsWithForbiddenRex("^[#(0-9]+");
    if (startsWithForbiddenRex.indexIn(name) >= 0)
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
        if (count > 10000)
            break; // this may create problems or crash lazyNut, but it prevents an infinite loop
    }
    return name;
}
