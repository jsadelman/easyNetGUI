#ifndef UI_DATACOMBOVIEWER_H
#define UI_DATACOMBOVIEWER_H

#include "ui_dataviewer.h"

class QComboBox;
class QScrollArea;

class Ui_DataComboViewer : public Ui_DataViewer
{
    Q_OBJECT
public:
    Ui_DataComboViewer(bool usePrettyNames);
    ~Ui_DataComboViewer();
    virtual QString currentItem();
    virtual void setCurrentItem(QString name);

public slots:
    virtual void addItem(QString name, QWidget *item) Q_DECL_OVERRIDE;
    virtual void removeItem(QString name) Q_DECL_OVERRIDE;
    virtual void replaceItem(QString name, QWidget *item) Q_DECL_OVERRIDE;


protected:
    virtual void createViewer();
    virtual void displayPrettyName(QString name);


private:
    QComboBox *comboBox;
    QScrollArea *scrollArea;
//    enum {ObjectNameRole = Qt::UserRole};
};

#endif // UI_DATACOMBOVIEWER_H
