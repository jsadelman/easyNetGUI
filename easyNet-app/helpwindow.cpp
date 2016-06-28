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
#include <QToolBar>
#include <QDesktopWidget>
#include <QSize>
#include <QFileDialog>
#include <QFileInfo>

#include "helpwindow.h"
#include "textedit.h"
#include "sessionmanager.h"


HelpWindow::HelpWindow()
{
    hToolBar = addToolBar("Help");
    hToolBar->setOrientation(Qt::Horizontal);

    htHome=hToolBar->addAction(QIcon(":/images/home.png"),tr("Home"));
    connect(htHome, SIGNAL(triggered()), this, SLOT(htHomeClicked()));

    htLoad=hToolBar->addAction(QIcon(":/images/open.png"),tr("Open html file"));
    connect(htLoad, SIGNAL(triggered()), this, SLOT(htLoadClicked()));

    htBack=hToolBar->addAction(QIcon(":/images/Back.png"),tr("Back"));
    connect(htBack, SIGNAL(triggered()), this, SLOT(back()));

    textViewer = new TextEdit;
    textViewer->setContents(SessionManager::instance()->defaultLocation("docsDir")+"/start.html");

    setCentralWidget(textViewer);
    setWindowTitle(tr("easyNet help"));


}

void HelpWindow::htHomeClicked()
{
    textViewer->setContents(SessionManager::instance()->defaultLocation("docsDir")+"/start.html");

}

void HelpWindow::htLoadClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load html page"),
                                                    SessionManager::instance()->defaultLocation("docsDir"),
                                                    tr("html Files (*.html);;htm Files (*.htm);;All files (*.*)"));
    if (!fileName.isEmpty())
    {
//        QLatin1String loc(fileName);
//        textViewer->setContents(fileName);
        QUrl localRef = QUrl::fromLocalFile(fileName);
//        textViewer->setSource(localRef); //can't figure out how to get QTextBrowser to understand relative references
        textViewer->setContents(fileName);
    }
    qDebug() << "loading html page:" << fileName;

//    QString searchPath = QFileInfo(fileName).absolutePath();
//    textViewer->setSearchPaths({searchPath});
//    qDebug() << "setting search path to" << searchPath;
}

void HelpWindow::back(){textViewer->back();}

void HelpWindow::showInfo(QString page)
{
    //    qDebug() << "show info" << page;
    textViewer->setContents(page);
}

