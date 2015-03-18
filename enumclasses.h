#ifndef ENUMCLASSES_H
#define ENUMCLASSES_H


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
    ListOfValues
};



#endif // ENUMCLASSES_H
