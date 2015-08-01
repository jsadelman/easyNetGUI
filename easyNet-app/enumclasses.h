#ifndef ENUMCLASSES_H
#define ENUMCLASSES_H

#include <Qt>
#include <QHash>

class AsLazyNutObject;
typedef QHash<QString,AsLazyNutObject*> LazyNutObjectCatalogue;

// model/view data roles
enum : unsigned int {ExpandToFillRole = Qt::UserRole};

// flags for logMode to be set up in LazyNutJobParam
enum : unsigned int
{
    ECHO_INTERPRETER    = 0x01,
};

enum class AnswerFormatterType
{
    NotInitialised,
    Identity,
    XML,
    ListOfValues,
    SVG
};



#endif // ENUMCLASSES_H
