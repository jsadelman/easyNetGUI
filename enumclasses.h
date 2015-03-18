#ifndef ENUMCLASSES_H
#define ENUMCLASSES_H

enum class SynchMode // : int
{
    Asynch,
    Synch
};

enum class JobOrigin
{
    User,
    GUI
};

// flags for logMode to be set up in LazyNutJobParam
enum : unsigned int
{
    ECHO_INTERPRETER    = 0x01,
};


#endif // ENUMCLASSES_H
