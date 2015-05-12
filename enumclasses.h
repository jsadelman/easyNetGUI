#ifndef ENUMCLASSES_H
#define ENUMCLASSES_H


class LazyNutObject;
typedef QHash<QString,LazyNutObject*> LazyNutObjectCatalogue;


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
