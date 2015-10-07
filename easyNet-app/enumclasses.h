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


#ifdef __APPLE__
  #define EN_FONT "Helvetica Neue"
#else
  #define EN_FONT "Georgia"
#endif

#ifdef __APPLE__
    #define EN_FONT_SMALL 12
#else
    #define EN_FONT_SMALL 10
#endif

#ifdef __APPLE__
    #define EN_FONT_MEDIUM 14
#else
    #define EN_FONT_MEDIUM 12
#endif

#ifdef __APPLE__
    #define EN_FONT_LARGE 18
#else
    #define EN_FONT_LARGE 16
#endif

#endif // ENUMCLASSES_H
