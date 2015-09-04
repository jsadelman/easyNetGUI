#ifndef SETTINGSFORMDIALOG_H
#define SETTINGSFORMDIALOG_H

#include <QDialog>

class QDomDocument;
class SettingsForm;


class SettingsFormDialog: public QDialog
{
public:
    SettingsFormDialog(QDomDocument* domDoc, SettingsForm* form, QString info, QWidget* parent=0);
    ~SettingsFormDialog();

public slots:
    virtual void accept() Q_DECL_OVERRIDE;


private:
    void build();

    QDomDocument* domDoc;
    SettingsForm* form;
    QString info;
};

#endif // SETTINGSFORMDIALOG_H
