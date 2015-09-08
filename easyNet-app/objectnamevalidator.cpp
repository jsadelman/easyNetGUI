#include "objectnamevalidator.h"
#include "objectcatalogue.h"

ObjectNameValidator::ObjectNameValidator(QObject *parent)
{
    forbiddenNames
                      << "\\s+"
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
}

QValidator::State ObjectNameValidator::validate(QString &input, int &pos) const
{
    Q_UNUSED(pos)
    QRegExp forbiddenRex(QString("^(%1)$").arg(forbiddenNames.join("|")));
    QModelIndexList objectMatchList = ObjectCatalogue::instance()->match(
                ObjectCatalogue::instance()->index(0,0),
                Qt::DisplayRole,
                input,
                1,
                Qt::MatchExactly);
    if (forbiddenRex.exactMatch(input) || objectMatchList.length() > 0)
        return Invalid;

    else
        return Acceptable;

}

bool ObjectNameValidator::isValid(QString input)
{
    int pos(0);
    return validate(input, pos) == QValidator::Acceptable;
}
