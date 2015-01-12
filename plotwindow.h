#ifndef PLOTWINDOW
#define PLOTWINDOW

#include <QSvgWidget>
#include <QMainWindow>
#include <QFile>
#include <QSize>

#include "editwindow.h"


class plotWindow : public QMainWindow
{
    Q_OBJECT

public:
    plotWindow(QWidget *parent = 0);
    QSvgWidget *plot_svg;
    CodeEditor      *textEdit;
//    bool maybeSave();
//    void setCurrentFile(const QString &fileName);
    int getValueFromByteArray(QByteArray ba, QString key);


//protected:
//    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void refreshSvg();
/*    void newFile();
    void open();
    bool save();
    bool saveAs();
    void documentWasModified();
*/

private:
    void createActions();
//    void createMenus();
    void createToolBars();
/*    void createStatusBar();
    void readSettings();
    void writeSettings();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
*/
    QString curFile;
    QByteArray plotByteArray;

    QMenu *fileMenu;
    QMenu *editMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *refreshAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *copyAct;

};


#endif // PLOTWINDOW

