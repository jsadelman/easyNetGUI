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

#include <QtWidgets/QApplication>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QDebug>
#include <QtWidgets/QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextBrowser>

#include "helpwindow.h"
#include "textedit.h"
#include <QDesktopWidget>
#include <QSize>

HelpWindow::HelpWindow(const QString &path, const QString &page,
                       QWidget *parent = 0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QRect screenSize = QDesktopWidget().availableGeometry(this);
    this->resize(QSize(screenSize.width() * 0.6f, screenSize.height() * 0.7f));

    textViewer = new QTextBrowser; // TextEdit;
//    QString loc(QString(path) + QString(page));
////    QString loc(":/documentation/siteexport/start.html");
//    textViewer->setContents(loc);

////    setCentralWidget(textViewer);
//    setWindowTitle(tr("easyNet help"));

    homeButton = new QPushButton(tr("&Home"));
    backButton = new QPushButton(tr("&Back"));
    closeButton = new QPushButton(tr("Close"));
    closeButton->setShortcut(tr("Esc"));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(homeButton);
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(textViewer);
    setLayout(mainLayout);

    connect(homeButton, SIGNAL(clicked()), textViewer, SLOT(home()));
    connect(backButton, SIGNAL(clicked()),
            textViewer, SLOT(backward()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(textViewer, SIGNAL(sourceChanged(const QUrl &)),
            this, SLOT(updateWindowTitle()));

    textViewer->setSearchPaths(QStringList() << path << ":/images");
//    textViewer->setSearchPaths(QStringList() << path );
    textViewer->setSource(page);

}

void HelpWindow::showPage(const QString &page)
{
    HelpWindow *browser = new HelpWindow(QString(":/documentation/siteexport/"),
                                         QString(page));
    browser->resize(500, 400);
    browser->show();
}

void HelpWindow::updateWindowTitle()
{
    qDebug() << textViewer->documentTitle();
//    qDebug() << path;
//    qDebug() << page;
    setWindowTitle(tr("Help: %1").arg(textViewer->documentTitle()));
}

//void HelpWindow::showInfo(QString page)
//{
////    qDebug() << "show info" << page;
//    textViewer->setContents(page);
//}

