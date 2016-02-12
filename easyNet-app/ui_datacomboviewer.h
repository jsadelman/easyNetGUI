#ifndef UI_DATACOMBOVIEWER_H
#define UI_DATACOMBOVIEWER_H

#include "ui_dataviewer.h"

class QComboBox;
class QScrollArea;
class QStackedWidget;

class Ui_DataComboViewer : public Ui_DataViewer
{
    Q_OBJECT
public:
    Ui_DataComboViewer();
    ~Ui_DataComboViewer();
    virtual QString currentItemName() Q_DECL_OVERRIDE;
    virtual QWidget *currentView() Q_DECL_OVERRIDE;

public slots:
    virtual void addView(QString name, QWidget *view) Q_DECL_OVERRIDE;
    virtual QWidget *takeView(QString name) Q_DECL_OVERRIDE;
    virtual void setCurrentItem(QString name) Q_DECL_OVERRIDE;


protected:
    virtual void createViewer();
    virtual void displayPrettyName(QString name);


private:
    QComboBox *comboBox;
    QScrollArea *scrollArea;
    QStackedWidget *stackedWidget;
//    enum {ObjectNameRole = Qt::UserRole};
};

#endif // UI_DATACOMBOVIEWER_H
