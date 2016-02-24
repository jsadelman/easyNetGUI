#ifndef SETTINGSFORMDIALOG_H
#define SETTINGSFORMDIALOG_H

#include <QDialog>

class QDomDocument;
class SettingsForm;
class QLineEdit;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;


class SettingsFormDialog: public QDialog
{
    Q_OBJECT
public:
    SettingsFormDialog(QDomDocument* domDoc, SettingsForm* form, QString info, QWidget* parent=0);
    ~SettingsFormDialog();
    void build();

public slots:
    virtual void accept() Q_DECL_OVERRIDE;

protected slots:
    virtual void checkBeforeAccept();

protected:
    virtual void addExtraWidgets() {}

    QDomDocument* domDoc;
    SettingsForm* form;
    QString info;

    QVBoxLayout* mainLayout;
    QHBoxLayout* topLayout;
    QLabel* infobLabel;
    QVBoxLayout* buttonsLayout;
    QPushButton* okButton;
    QPushButton* cancelButton;




};

#endif // SETTINGSFORMDIALOG_H
