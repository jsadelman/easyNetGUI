#ifndef DROPLINEEDIT_H
#define DROPLINEEDIT_H

#include <QLineEdit>

class DropLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    DropLineEdit(QWidget *parent = Q_NULLPTR);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
};

#endif // DROPLINEEDIT_H
