/****************************************************************************
**
**
****************************************************************************/

#include <QtWidgets>
#include <QSvgWidget>


#include "plotwindow.h"
#include "codeeditor.h"
#include "lazynutjobparam.h"

#include "easyNetMainWindow.h"
#include "sessionmanager.h"

PlotWindow::PlotWindow(QWidget *parent)
    : QMainWindow(parent)

{
//    textEdit = new QPlainTextEdit;
    plot_svg = new QSvgWidget(this);
//    plot_svg->load(QString(":/images/test.svg"));
//    plot_svg->load(QString("C:/Users/colind/Documents/Top/easyNET_GUI/lazyNutGUI/images/test.svg"));

    // Get the file name using a QFileDialog
//    QFile file(QFileDialog::getOpenFileName(NULL, tr("Upload a file")));
//    QFile file("C:/Users/mm14722/Dropbox/scambio_temp/work/Qt/parser/tree_view/tree_view/images/test.svg");
    QFile file(":/images/test.svg");

    int width, height;

    // If the selected file is valid, continue with the upload
//    if (!file.fileName().isEmpty)
    if (file.open(QIODevice::ReadOnly))
    {
        // Read the file and transform the output to a QByteArray
        plotByteArray = file.readAll();
        plotByteArray.replace (QByteArray("<symbol"),QByteArray("<g     "));
        plotByteArray.replace (QByteArray("</symbol"),QByteArray("</g     "));
        width = getValueFromByteArray(plotByteArray, "width");
        height = getValueFromByteArray(plotByteArray, "height");
        if (width<1)
            width = 500;
        if (height<1)
            height = 500;
        QSize size(width,height); // plot_svg->sizeHint();
//        size.scale(1200, 400); // , Qt::KeepAspectRatio);
        plot_svg->setMaximumSize(size);
        plot_svg->setMinimumSize(size);
//        plot_svg->setWi

        // Send the QByteArray
        plot_svg->load(plotByteArray);
    }

    textEdit = new CodeEditor(this);
    setCentralWidget(plot_svg);

    createActions();
//    createMenus();
    createToolBars();
//    createStatusBar();

//    readSettings();

/*
 *     connect(textEdit->document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));
*/
//    setCurrentFile("");
    setUnifiedTitleAndToolBarOnMac(true);
}

int PlotWindow::getValueFromByteArray(QByteArray ba, QString key)
{
    QMessageBox msgBox;
    QByteArray w;
    int value;
    bool ok;
    int p = ba.indexOf(key, 0);
    if (p>-1)
    {
        w = ba.mid(p+key.size()+2, 3);
        value = w.toInt(&ok, 10);
        if (!ok)
        {
            QString printable = QString("Problem reading value. String was %1").arg(QString(w));
            msgBox.setText(printable);
             msgBox.exec();
        }
/*
 *         else
        {
            QString printable = QString("%1 is %2. String was %3").arg(key).arg(value).arg(QString(w));
            msgBox.setText(printable);
            msgBox.exec();
        }
*/
    }
    return(value);
}


void PlotWindow::createActions()
{
    refreshAct = new QAction(QIcon(":/images/reload.png"), tr("&Refresh"), this);
    refreshAct->setShortcuts(QKeySequence::Refresh);
//    refreshAct->setStatusTip(tr("Refresh plot"));
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refreshSvg()));

/*
      openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
//    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));


    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
//    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
//    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
//    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
//                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));

*/
}

/*
void plotWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    if (cutAllowed)
        editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    if (pasteAllowed)
        editMenu->addAction(pasteAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}
*/

void PlotWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(refreshAct);
//        fileToolBar->addAction(openAct);
//    fileToolBar->addAction(saveAct);

//    editToolBar = addToolBar(tr("Edit"));
//    editToolBar->addAction(copyAct);
}

void PlotWindow::refreshSvg()
{
// This function sends a "plo get" command to the inpterpreter,
// reads the output into a byteArray, does some processing on it
// then loads it into the svgWidget

    LazyNutJobParam *param = new LazyNutJobParam;
    param->cmdList = {"plo get"};
    param->answerFormatterType = AnswerFormatterType::SVG;
    param->setAnswerReceiver(this, SLOT(displaySVG(int, QByteArray)));
    qDebug() << "Address of object is " << parent();
//    qDebug() << "Name of object is " << (qobject_cast<EasyNetMainWindow *> (this->parent()))->objectName();
//    (qobject_cast<EasyNetMainWindow *> (parent()))->sessionManager->setupJob(param);
}

void PlotWindow::displaySVG(int sizeArray, QByteArray plotByteArray)
{

//    read number bytes from interpreter into numBytes, then resize array accordingly
//    plotByteArray.resize(sizeArray);

//    replace tags that QSvgWidget doesn't like
    plotByteArray.replace (QByteArray("<symbol"),QByteArray("<g     "));
    plotByteArray.replace (QByteArray("</symbol"),QByteArray("</g     "));

//    get dimensions and set plot size accordingly
    int width = getValueFromByteArray(plotByteArray, "width");
    int height = getValueFromByteArray(plotByteArray, "height");
    if (width<1)
        width = 500;
    if (height<1)
        height = 500;
    QSize size(width,height); // plot_svg->sizeHint();
    plot_svg->setMaximumSize(size);
    plot_svg->setMinimumSize(size);

//    load the byte array into the plot
    plot_svg->load(plotByteArray);

}
