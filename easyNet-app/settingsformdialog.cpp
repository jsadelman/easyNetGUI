#include "settingsformdialog.h"
#include "settingsform.h"

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


SettingsFormDialog::SettingsFormDialog(QDomDocument *domDoc, SettingsForm *form, QString info, QWidget *parent)
    : domDoc(domDoc), form(form), info(info), QDialog(parent)
{
}

SettingsFormDialog::~SettingsFormDialog()
{
    delete form;
}

void SettingsFormDialog::accept()
{
    settings = form->getSettings();
//    QMapIterator<QString, QString> settings_it(settings);
//    while (settings_it.hasNext())
//    {
//        settings_it.next();
//        form->setSetting(settings_it.key(), settings_it.value());
//    }
    QDialog::accept();
}

void SettingsFormDialog::checkBeforeAccept()
{
    accept();
}

void SettingsFormDialog::build()
{
    mainLayout = new QVBoxLayout;
    topLayout = new QHBoxLayout;
    infobLabel = new QLabel(info,this);
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
    buttonsLayout = new QVBoxLayout;
    okButton = new QPushButton("Ok");
    cancelButton = new QPushButton("Cancel");
    connect(okButton, SIGNAL(clicked()), this, SLOT(checkBeforeAccept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    topLayout->addWidget(infobLabel);
    topLayout->addStretch();
    topLayout->addLayout(buttonsLayout);
    mainLayout->addLayout(topLayout);
    addExtraWidgets();
    setLayout(mainLayout);
    form->build();
    mainLayout->addWidget(form);
}

