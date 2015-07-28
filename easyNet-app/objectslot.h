#ifndef OBJECTSLOT_H
#define OBJECTSLOT_H

#include <QPair>


class ObjectSlot : protected QPair
{
public:
    explicit ObjectSlot(QObject* obj, char const * slot): QPair(obj, slot) {}


};

#endif // OBJECTSLOT_H
