#include "dataframemergesettingsformdialog.h"
#include "settingsform.h"
#include "sessionmanager.h"


#include <QDomDocument>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QDebug>

DataframeMergeSettingsFormDialog::DataframeMergeSettingsFormDialog(QDomDocument* domDoc, SettingsForm* form, QString info, QWidget* parent)
    : SettingsFormDialog(domDoc, form, info, parent)
{
}


void DataframeMergeSettingsFormDialog::accept()
{
    QStringList cmdList;
    QString dfName = nameLineEdit->text();
    cmdList << QString("create dataframe_merge %1").arg(dfName);
    cmdList << form->getSettingsCmdList().replaceInStrings(QRegExp("^"), QString("%1 set_").arg(dfName));
    emit dataframeMergeSettingsReady(cmdList, dfName, form->value("x"), form->value("y"));
    SettingsFormDialog::accept();
}


void DataframeMergeSettingsFormDialog::addExtraWidgets()
{
    QFormLayout *nameLayout = new QFormLayout;
    nameLineEdit = new QLineEdit;
    connect(nameLineEdit, SIGNAL(editingFinished()), this, SLOT(validateName()));
    nameLayout->addRow("Dataframe name:", nameLineEdit);
    mainLayout->addLayout(nameLayout);
}

void DataframeMergeSettingsFormDialog::validateName()
{
    if (!SessionManager::instance()->isValidObjectName(nameLineEdit->text()))
    {
        QMessageBox::critical(this, "Illegal dataframe name",QString("The name you chose is not valid, since it conflicts with an existing name or command.\n"
                                                                     "Please select another name or keep the suggested name"));
        nameLineEdit->setText(SessionManager::instance()->makeValidObjectName(nameLineEdit->text()));
    }
}

void DataframeMergeSettingsFormDialog::checkBeforeAccept()
{
    if (form->allIsSet() && !nameLineEdit->text().isEmpty())
        accept();

    else
    {
        QMessageBox::critical(this, "Missing values",QString("Please fill in all the values in this form."));
    }
}

