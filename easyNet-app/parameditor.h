#ifndef PARAMEDITOR_H
#define PARAMEDITOR_H

#include <QMainWindow>
class QTableView;

class ParamEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit ParamEditor(QWidget *parent = 0);

private:
    QTableView     *view;

};

#endif // PARAMEDITOR_H
