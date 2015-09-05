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
    // just print for the moment
//    QStringList settingsList = form->getSettingsCmdList();
    QStringList cmdList;
    QString dbName = nameLineEdit->text();
    cmdList << QString("create dataframe_merge %1").arg(dbName);
    cmdList << form->getSettingsCmdList().replaceInStrings(QRegExp("^"), QString("%1 set_").arg(dbName));
    cmdList.replaceInStrings(QRegExp("(set_[xy]_key) (.*)$"), "\\1 \"\\2\"");
    cmdList << QString ("%1 get").arg(dbName);
//    qDebug() << cmdList;

    emit cmdListReady(cmdList);

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
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
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
    nameLayout->addRow("Dataframe name:", nameLineEdit);
    mainLayout->addLayout(nameLayout);
    // form
    QScrollArea *scrollArea = new QScrollArea;
    form->build();
    scrollArea->setWidget(form);
    mainLayout->addWidget(scrollArea);
    setLayout(mainLayout);
}

