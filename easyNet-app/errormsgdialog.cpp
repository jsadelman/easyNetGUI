#include "errormsgdialog.h"

#include <QApplication>
#include <QLabel>
#include <QStyle>
#include <QScrollArea>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QBoxLayout>

void ErrorMsgDialog::build()
{
    setWindowTitle("Simulation Engine Error");
    QLabel *warningLabel = new QLabel;
    int iconSize = qApp->style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
    warningLabel->setPixmap(QIcon(qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning)).pixmap(iconSize));
    warningLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QLabel *headerLabel = new QLabel("The last command(s) triggered error(s).");
    headerLabel->setWordWrap(true);
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(warningLabel);
    topLayout->addWidget(headerLabel);

    QScrollArea *scrollArea = new QScrollArea;
    errorLogDisplay = new QTextEdit;
    errorLogDisplay->setReadOnly(true);
    errorLogDisplay->setFontFamily("Courier");
    scrollArea->setWidget(errorLogDisplay);
    scrollArea->setWidgetResizable(true);
    QLabel *footerLabel = new QLabel("Would you like to save the simulator logs?");
    footerLabel->setWordWrap(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);
    buttonBox->button(QDialogButtonBox::Yes)->setAutoDefault(false);
    buttonBox->button(QDialogButtonBox::No)->setDefault(true);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(footerLabel);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    //    QFontMetrics fm(QApplication::font("QMessageBox"));
    //    int iconHeight = fm.height();
    //    QString errorInfo = QString("<p>Note: a detailed log of commands and errors can always be retrieved from the Expert window "
    //                      "(<img src=':/images/expert.mode.jpg' height=%1> button).</p>").arg(iconHeight);

}

ErrorMsgDialog::ErrorMsgDialog(QWidget *parent)
    : QDialog(parent)
{
    build();
}

void ErrorMsgDialog::setErrorLog(QString errorLog)
{
    errorLogDisplay->setText(errorLog);
}
