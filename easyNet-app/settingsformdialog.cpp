#include "settingsformdialog.h"
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

SettingsFormDialog::SettingsFormDialog(QDomDocument* domDoc, SettingsForm* form, QString info, QWidget* parent)
    : domDoc(domDoc), form(form), info(info), QDialog(parent)
{
    build();
}

SettingsFormDialog::~SettingsFormDialog()
{
    delete form;
//    delete domDoc;
}

void SettingsFormDialog::accept()
{
    QStringList cmdList;
    QString dfName = nameLineEdit->text();
    cmdList << QString("create dataframe_merge %1").arg(dfName);
    cmdList << form->getSettingsCmdList().replaceInStrings(QRegExp("^"), QString("%1 set_").arg(dfName));
    emit dataframeMergeSettingsReady(cmdList, dfName, form->value("x"), form->value("y"));
    QDialog::accept();
}

void SettingsFormDialog::build()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    // top
    QHBoxLayout* topLayout = new QHBoxLayout;
    QLabel* infobLabel = new QLabel(info,this);
    infobLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    infobLabel->setWordWrap(true);
    QString backCol("white");
    QString textCol("black");
    infobLabel->setStyleSheet("QLabel {"
                            "background-color: " + backCol + ";"
                            "color: " + textCol + ";"
                             "border: 1px solid black;"
                             "padding: 4px;"
                             "font: bold 12pt;"
                             "}");
    QVBoxLayout* buttonsLayout = new QVBoxLayout;
    QPushButton* okButton = new QPushButton("Ok");
    QPushButton* cancelButton = new QPushButton("Cancel");
    connect(okButton, SIGNAL(clicked()), this, SLOT(checkBeforeAccept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    topLayout->addWidget(infobLabel);
    topLayout->addStretch();
    topLayout->addLayout(buttonsLayout);
    mainLayout->addLayout(topLayout);
    // db name
    QFormLayout *nameLayout = new QFormLayout;
    nameLineEdit = new QLineEdit;
    connect(nameLineEdit, SIGNAL(editingFinished()), this, SLOT(validateName()));
    nameLayout->addRow("Dataframe name:", nameLineEdit);
    mainLayout->addLayout(nameLayout);
    // form
//    QScrollArea *scrollArea = new QScrollArea;

//    mainLayout->addWidget(scrollArea);
    setLayout(mainLayout);
    form->build();
     mainLayout->addWidget(form);
//    scrollArea->setWidget(form);

}

void SettingsFormDialog::validateName()
{
    if (!SessionManager::instance()->isValidObjectName(nameLineEdit->text()))
    {
        QMessageBox::critical(this, "Illegal dataframe name",QString("The name you chose is not valid, since it conflicts with an existing name or command.\n"
                                                                     "Please select another name."));
        nameLineEdit->setText("");
    }
}

void SettingsFormDialog::checkBeforeAccept()
{
    if (form->allIsSet() && !nameLineEdit->text().isEmpty())
        accept();

    else
    {
        QMessageBox::critical(this, "Missing values",QString("Please fill in all the values in this form."));
    }
}

