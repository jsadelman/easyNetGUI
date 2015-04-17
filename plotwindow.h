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
    void sendDrawCmd();
    void displaySVG(QByteArray plotByteArray);
    void dumpSVG(QString svg);
    void setPlot(QString name);
    void newPlot();
    void createNewPlot(QString name);
    void setType(QString rScript);
    void getSettingsXML();
    void buildSettingsForm(QDomDocument* settingsList);
    void sendSettings(QObject *nextJobReceiver = nullptr, char const *nextJobSlot = "");
    void updateSettingsForm();
    void selectRScript();
    void selectRecentRScript();
    void setCurrentPlotType(QString rScript);
    void draw();


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
    LazyNutListMenu *plotsMenu;
    QAction *drawAct;
    QAction *separatorAct;
    enum { MaxRecentRScripts = 5 };
    QAction *recentRScriptsActs[MaxRecentRScripts];
    QAction *selectRScriptAct;
    QAction *selectDataAct;

    QString currentPlotType;
    QString currentOutput;
    QString currentPlot;
    QString createNewPlotText;

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

