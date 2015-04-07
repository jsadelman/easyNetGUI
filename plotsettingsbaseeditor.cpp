#include <QtWidgets>
#include "plotsettingsbaseeditor.h"

PlotSettingsBaseEditor::PlotSettingsBaseEditor(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    // header
    QWidget *headerBox = new QWidget(this);
    QHBoxLayout *headerBoxLayout = new QHBoxLayout;
    nameLabel = new QLabel("no-name", this);
    QPushButton *commentButton = new QPushButton("?", this);
    connect(commentButton, SIGNAL(clicked()), this, SLOT(displayComment()));
    valueEdit = new QLineEdit(this);

    headerBoxLayout->addWidget(nameLabel);
    headerBoxLayout->addWidget(commentButton);
    headerBoxLayout->addStretch();
    headerBoxLayout->addWidget(valueEdit);
    headerBox->setLayout(headerBoxLayout);

    vBoxLayout->addWidget(headerBox);
    setLayout(vBoxLayout);
}

QString PlotSettingsBaseEditor::getValue()
{
    return valueEdit->text().simplified();
}

void PlotSettingsBaseEditor::setName(QString name)
{
    nameLabel->setText(name);
}

void PlotSettingsBaseEditor::setValue(QVariant v)
{
    valueEdit->setText(v.toString());
}

void PlotSettingsBaseEditor::displayComment()
{
    QMessageBox::information(this, "Setting description", m_comment);
}
