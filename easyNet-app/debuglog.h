#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#include <QDialog>
#include <QMainWindow>
#include <QTextDocument>

QT_BEGIN_NAMESPACE
class QToolBar;
class QDialogButtonBox;
class QTableView;
class QModelIndex;
class QStandardItemModel;
class QSortFilterProxyModel;
class FindDialog;

QT_END_NAMESPACE

class DebugLog : public QMainWindow
{
    Q_OBJECT

public:
    explicit DebugLog(QWidget *parent = 0);
    ~DebugLog();

private slots:
    void addRowToTable(QString cmd);
    void updateCmd(QString cmd, QString time);
    void on_copy_clicked();
    void showFindDialog();
    void findForward(const QString &str, QFlags<QTextDocument::FindFlag> flags);
    void save();
    void autoSave() {saveLogToFile(defaultLogFileName());}

private:
    void createToolBars();
    void createActions();
    void init(const QString &tableName, QWidget *parent);
    void setViewToStringList();
    QString defaultLogFileName();
    void saveLogToFile(QString fileName);

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *saveAct;
    QAction *copyAct;
    QAction *findAct;

    QStandardItemModel *model;
    QSortFilterProxyModel *proxyModel;
    QTableView      *view;
    FindDialog*     findDialog;

    int     lastExecCmdRow;
};


#endif // DEBUGLOG_H
