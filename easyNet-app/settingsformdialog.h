#ifndef SETTINGSFORMDIALOG_H
#define SETTINGSFORMDIALOG_H

#include <QDialog>

class QDomDocument;
class SettingsForm;
class QLineEdit;
class ObjectNameValidator;


class SettingsFormDialog: public QDialog
{
    Q_OBJECT
public:
    // This class should become a base class. Now it's specific to merge dataframes.
    SettingsFormDialog(QDomDocument* domDoc, SettingsForm* form, QString info, QWidget* parent=0);
    ~SettingsFormDialog();

public slots:
    virtual void accept() Q_DECL_OVERRIDE;

signals:
    void cmdListReady(QStringList);
    void dfNameReady(QString, QString, QString);

private slots:
    void validateName();
    void checkBeforeAccept();


private:
    void build();

    QDomDocument* domDoc;
    SettingsForm* form;
    QString info;
    QLineEdit *nameLineEdit;
    ObjectNameValidator *validator;
};

#endif // SETTINGSFORMDIALOG_H
