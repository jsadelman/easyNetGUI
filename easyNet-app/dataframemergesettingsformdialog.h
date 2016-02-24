#ifndef DATAFRAMEMERGESETTINGSFORMDIALOG_H
#define DATAFRAMEMERGESETTINGSFORMDIALOG_H

#include "settingsformdialog.h"

class QDomDocument;
class SettingsForm;
class QLineEdit;
class ObjectNameValidator;


class DataframeMergeSettingsFormDialog: public SettingsFormDialog
{
    Q_OBJECT
public:
    // This class should become a base class. Now it's specific to merge dataframes.
    DataframeMergeSettingsFormDialog(QDomDocument* domDoc, SettingsForm* form, QString info, QWidget* parent=0);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;

signals:
    void dataframeMergeSettingsReady(QStringList, QString, QString, QString);

protected slots:
    void validateName();
    virtual void checkBeforeAccept() Q_DECL_OVERRIDE;


protected:
    virtual void addExtraWidgets() Q_DECL_OVERRIDE;

    QLineEdit *nameLineEdit;
};

#endif // DATAFRAMEMERGESETTINGSFORMDIALOG_H
