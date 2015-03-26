#ifndef PLOTWINDOW
#define PLOTWINDOW

#include <QMainWindow>
#include <QGroupBox>

class QSvgWidget;
class CodeEditor;
class QDomDocument;
class QLabel;
class QPushButton;
class QDoubleSpinBox;
class QSpinBox;
class QVBoxLayout;

class NumericSettingsForPlotWidget: public QGroupBox
{
    Q_OBJECT

public:
    NumericSettingsForPlotWidget(QString name, QString value, QString comment, QString defaultValue, QWidget *parent = 0);
    QString getValue() {return value;}
    QString getName() {return name;}

private slots:
    void displayComment();
    void setValue(QString val) {value = val;}

private:
    QString name;
    QString value;
    QString comment;
    QString defaultValue;
    QLabel *nameLabel;
    QPushButton *commentButton;
    QDoubleSpinBox *doubleSpinBox;
    QSpinBox *intSpinBox;

};



class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    PlotWindow(QWidget *parent = 0);
//    bool maybeSave();
//    void setCurrentFile(const QString &fileName);
    int getValueFromByteArray(QByteArray ba, QString key);

signals:
    void sendPloGet();

//protected:
//    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void refreshSvg();
    void displaySVG(QByteArray plotByteArray);
    void dumpSVG(QString svg);
    void setType(QString rScript);
    void listSettings();
    void buildPlotControlPanel(QDomDocument* settingsList);
    void selectRScript();
    void selectRecentRScript();
    void setCurrentPlotType(QString rScript);
    void selectOutput();
    void redraw();

/*    void newFile();
    void open();
    bool save();
    bool saveAs();
    void documentWasModified();
*/

private:


    void createPlotControlPanel();
    void createActions();
    void updateRecentRScriptsActs();
//    void createMenus();
    void createToolBars();
/*    void createStatusBar();
    void readSettings();
    void writeSettings();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
*/
    QDialog *plotControlPanel;
    QVBoxLayout *plotControlPanelLayout;
    QSvgWidget *plot_svg;

    QList<NumericSettingsForPlotWidget*> numericSettingsWidgets;
    QMenu *typeMenu;
    QMenu *recentRScriptsMenu;
    QMenu *contentMenu;
    QAction *separatorAct;
    enum { MaxRecentRScripts = 5 };
    QAction *recentRScriptsActs[MaxRecentRScripts];
    QAction *selectRScriptAct;
    QAction *selectOutputAct;

    QString currentPlotType;
    QString currentOutput;

    QString curFile;
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

