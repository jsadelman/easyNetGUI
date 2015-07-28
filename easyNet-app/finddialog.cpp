/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include "finddialog.h"

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    label = new QLabel(tr("Find &what:"));
    lineEdit = new QLineEdit;
    label->setBuddy(lineEdit);

    caseCheckBox = new QCheckBox(tr("Match &case"));
//    fromStartCheckBox = new QCheckBox(tr("Search from &start"));
//    fromStartCheckBox->setChecked(true);

    findButton = new QPushButton(tr("&Find"));
    findButton->setDefault(true);

//    moreButton = new QPushButton(tr("&More"));
//    moreButton->setCheckable(true);
//    moreButton->setAutoDefault(false);

    extension = new QWidget;

    wholeWordsCheckBox = new QCheckBox(tr("&Whole words"));
    backwardCheckBox = new QCheckBox(tr("Search &backward"));
    searchSelectionCheckBox = new QCheckBox(tr("Search se&lection"));

    buttonBox = new QDialogButtonBox(Qt::Vertical);
    buttonBox->addButton(findButton, QDialogButtonBox::ActionRole);
//    buttonBox->addButton(moreButton, QDialogButtonBox::ActionRole);

//    connect(moreButton, SIGNAL(toggled(bool)), extension, SLOT(setVisible(bool)));

    QVBoxLayout *extensionLayout = new QVBoxLayout;
    extensionLayout->setMargin(0);
    extensionLayout->addWidget(caseCheckBox);
    extensionLayout->addWidget(wholeWordsCheckBox);
    extensionLayout->addWidget(backwardCheckBox);
//    extensionLayout->addWidget(searchSelectionCheckBox);
    extension->setLayout(extensionLayout);

    QHBoxLayout *topLeftLayout = new QHBoxLayout;
    topLeftLayout->addWidget(label);
    topLeftLayout->addWidget(lineEdit);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addLayout(topLeftLayout);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addLayout(leftLayout, 0, 0);
    mainLayout->addWidget(buttonBox, 0, 1);
    mainLayout->addWidget(extension, 1, 0, 1, 2);
    mainLayout->setRowStretch(2, 1);

    setLayout(mainLayout);

    setWindowTitle(tr("Find text"));

    connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(enableFindButton(const QString &)));
    connect(findButton, SIGNAL(clicked()), this, SLOT(findClicked()));
//    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

}

void FindDialog::hideExtendedOptions()
{
    extension->hide();
}

/*
FindDialog::FindDialog(QWidget *parent = 0):QDialog(parent)
{
   //Creating components
   label = new QLabel(tr("Find &what : "));
   lineEdit = new QLineEdit;
   label->setBuddy(lineEdit);

   caseCheckBox = new QCheckBox(tr("Match &case"));
   backwardCheckBox = new QCheckBox(tr("Search &backward"));
   findButton = new QPushButton(tr("&Find"));
   findButton->setDefault(true);
   findButton->setEnabled(false);
   closeButton = new QPushButton(tr("close"));
   //adding event handling
   connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(enableFindButton(const QString &)));
   connect(findButton, SIGNAL(clicked()), this, SLOT(findClicked()));
   connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
   //setting layout
   QHBoxLayout *topLeftLayout = new QHBoxLayout;
   topLeftLayout->addWidget(label);
   topLeftLayout->addWidget(lineEdit);

   QVBoxLayout *leftLayout = new QVBoxLayout;
   leftLayout->addLayout(topLeftLayout);
   leftLayout->addWidget(caseCheckBox);
   leftLayout->addWidget(backwardCheckBox);

   QVBoxLayout *rightLayout = new QVBoxLayout;
   rightLayout->addWidget(findButton);
   rightLayout->addWidget(closeButton);
   rightLayout->addStretch();

   QHBoxLayout *mainLayout = new QHBoxLayout;
   mainLayout->addLayout(leftLayout);
   mainLayout->addLayout(rightLayout);
   setLayout(mainLayout);
   //window title
   setWindowTitle(tr("Find"));
   setFixedHeight(sizeHint().height());
  }
*/


 void FindDialog::findClicked()
 {
   QString text = lineEdit->text();
   QFlags<QTextDocument::FindFlag> flags;
   if(backwardCheckBox->isChecked())
     flags |= QTextDocument::FindBackward;
   if(caseCheckBox->isChecked())
     flags |= QTextDocument::FindCaseSensitively;
   if(wholeWordsCheckBox->isChecked())
     flags |= QTextDocument::FindWholeWords;
   if (backwardCheckBox->isChecked())
      emit findBackward(text, flags);
   else
      emit findForward(text, flags);
 }

 void FindDialog::enableFindButton(const QString &text)
 {
   findButton->setEnabled(!text.isEmpty());
 }


