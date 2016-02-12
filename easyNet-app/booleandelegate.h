#ifndef BOOLEANDELEGATE_H
#define BOOLEANDELEGATE_H

#include <QStyledItemDelegate>


class BooleanDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    BooleanDelegate();
    ~BooleanDelegate();
};

#endif // BOOLEANDELEGATE_H
