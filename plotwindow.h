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
class QListView;
class QScrollArea;
class PlotSettingsForm;
class LazyNutListMenu;



class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    PlotWindow(QWidget *parent = 0);
    int getValueFromByteArray(QByteArray ba, QString key);

signals:
    void sendPloGet();

private slots:
    void refreshSvg();
    void displaySVG(QByteArray plotByteArray);
    void dumpSVG(QString svg);
    void setType(QString rScript);
    void setDataframe(QString dataframe);
    void listSettings();
    void buildPlotControlPanel(QDomDocument* settingsList);
    void selectRScript();
    void selectRecentRScript();
    void setCurrentPlotType(QString rScript);
    void redraw();


private:


    void createPlotControlPanel();
    void createActions();
    void updateRecentRScriptsActs();
    void createToolBars();

    QMainWindow *plotControlPanelWindow;
    QScrollArea *plotControlPanelScrollArea;
    QVBoxLayout *plotControlPanelLayout;
    QSvgWidget *plot_svg;

    PlotSettingsForm *plotSettingsForm;

    QMenu *typeMenu;
    QMenu *recentRScriptsMenu;
    LazyNutListMenu *dataMenu;
    QAction *redrawAct;
    QAction *separatorAct;
    enum { MaxRecentRScripts = 5 };
    QAction *recentRScriptsActs[MaxRecentRScripts];
    QAction *selectRScriptAct;
    QAction *selectDataAct;

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

