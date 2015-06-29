#include "dataframewindow.h"
#include "tableeditor.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"

#include <QtDebug>

DataFrameWindow::DataFrameWindow(QWidget *parent)
{
//    editor = new TableEditor ("tbl",this);
//    editor->setMinimumWidth(this->width());
//    editor->setMinimumHeight(this->height());
//    editor->show();
//    testDataFrame();
}

DataFrameWindow::~DataFrameWindow()
{

}

void DataFrameWindow::testDataFrame()
{
QStringList *headerList;
headerList = new QStringList();
*headerList << "stimulus" << "Frequency" << "N";

QStringList col1;
QStringList col2;
QStringList col3;
col1 << "side" <<	"take" <<	"hold" <<	"plan" <<	"rich" <<	"soft" <<	"rink" <<	"lust" <<	"mink" <<	"tact" <<	"yoke" <<	"rasp";
col2 << "High" << "High" << "High" << "High" << "High" << "High" << "Low" << "Low" << "Low" << "Low" << "Low" << "Low";
col3 << "Large" << "Large" << "Large" << "Small" << "Small" << "Small" << "Large" << "Large" << "Large" << "Small" << "Small" << "Small";
QList<QStringList> *myData = new QList<QStringList>;
*myData << col1 << col2 << col3;
QString name = "Andrews_92_short";

//    editor->AddDataFrame(name, headerList, myData);
}

void DataFrameWindow::getDataFrameHeaders(QString df_name)
{
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({"xml " + df_name + " get_colnames"});
//    param->cmdList = QStringList({"xml " + df_name + " get_colwise"});
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(this, SLOT(processList(QStringList)));
    param->setNextJobReceiver(this, SLOT(getDataFrameContents()));
    SessionManager::instance()->setupJob(param, sender());
}

void DataFrameWindow::processList(QStringList headerList)
{
    qDebug() << "Processing list" << headerList;

}

void DataFrameWindow::getDataFrameContents()
{
    QString df_name = "Andrews_92_short";
    qDebug() << "Entered getDataFrameContents";
    LazyNutJobParam *param = new LazyNutJobParam;
    param->logMode |= ECHO_INTERPRETER; // debug purpose
    param->cmdList = QStringList({"xml " + df_name + " get_colwise"});
    param->answerFormatterType = AnswerFormatterType::ListOfValues;
    param->setAnswerReceiver(this, SLOT(processData(QList(QStringList))));
    SessionManager::instance()->setupJob(param, sender());
}

void DataFrameWindow::processData(QList<QStringList> dataList)
{
    qDebug() << "Processing data" << dataList;

}

void DataFrameWindow::AddDataFrame(QString name, QStringList *headerList, QList<QStringList> *myData)
{
//    editor->AddDataFrame(name, headerList, myData);
}



