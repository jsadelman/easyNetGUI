#ifndef DATAFRAMEWINDOW_H
#define DATAFRAMEWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>


class TableEditor;

class DataFrameWindow : public QMainWindow
{
    Q_OBJECT

public:
    DataFrameWindow(QWidget *parent);
    ~DataFrameWindow();

    void getDataFrameHeaders(QString df_name);
    void getDataFrameContents();
    void testDataFrame();

private:
    TableEditor *editor;

private slots:
    void processList(QStringList headerList);
    void processData(QList<QStringList> dataList);
    void AddDataFrame(QString name, QStringList *headerList, QList<QStringList> *myData);
};



#endif // DATAFRAMEWINDOW_H
