#include "settingsformdialog.h"
#include "settingsform.h"

#include <QDomDocument>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QDebug>

SettingsFormDialog::SettingsFormDialog(QDomDocument* domDoc, SettingsForm* form, QString info, QWidget* parent)
    : domDoc(domDoc), form(form), info(info), QDialog(parent)
{
    build();
}

SettingsFormDialog::~SettingsFormDialog()
{
    delete form;
    delete domDoc;
}

void SettingsFormDialog::accept()
{
    // just print for the moment
    qDebug() << form->getSettingsCmdList();
}

void SettingsFormDialog::build()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    // top
    QHBoxLayout* topLayout = new QHBoxLayout;
    QLabel* infobLabel = new QLabel(info,this);
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
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    topLayout->addWidget(infobLabel);
    topLayout->addStretch();
    topLayout->addLayout(buttonsLayout);
    mainLayout->addLayout(topLayout);
    // form
    QScrollArea *scrollArea = new QScrollArea;
    form->build();
    scrollArea->setWidget(form);
    mainLayout->addWidget(scrollArea);
    setLayout(mainLayout);
}

