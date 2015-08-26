#ifndef PLOTWINDOW
#define PLOTWINDOW

#include <QMainWindow>
#include <QWizard>
#include <QMap>


class CodeEditor;
class QDomDocument;
class QLabel;
class QLineEdit;
class QPushButton;
class QDoubleSpinBox;
class QSpinBox;
class QVBoxLayout;
class QListView;
class QScrollArea;
class PlotSettingsForm;
class LazyNutListMenu;

class NewPlotWizard: public QWizard
{
    Q_OBJECT

public:
    NewPlotWizard(QWidget *parent = 0);
    void accept() Q_DECL_OVERRIDE;

signals:
    void createNewPlotOfType(QString, QString);

};

class NewPlotPage: public QWizardPage
{
    Q_OBJECT

public:
    NewPlotPage(QWidget *parent = 0);

private slots:
    void selectRScript();

private:
    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QLabel *typeLabel;
    QLineEdit *typeEdit;
    QPushButton *browseButton;
};


class PlotSettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    PlotSettingsWindow(QWidget *parent = 0);
    int getValueFromByteArray(QByteArray ba, QString key);

    void setDefaultModelSetting(QString setting, QString value);
signals:
        void plot(QString, QByteArray);
        void newPlotSignal(QString);
public slots:
        void createNewPlotOfType(QString name, QString type,
                                 QMap<QString, QString> _defaultSettings=QMap<QString,QString>());

private slots:
    void sendDrawCmd(QString plotName);
    void displaySVG(QByteArray plotByteArray, QString cmd);
    void setPlot(QString name);
    void newPlot();
    void createNewPlot(QString name);
    void setType(QString rScript);
    void getSettingsXML();
    void buildSettingsForm(QDomDocument* settingsList);
    void sendSettings(QObject *nextJobReceiver = nullptr, char const *nextJobSlot = "");
    void getPlotType(QObject *nextJobReceiver = nullptr, char const *nextJobSlot = "");
    void extractPlotType(QDomDocument* description);
//    void updateSettingsForm();
    void selectRScript();
    void selectRecentRScript();
    void setCurrentPlotType(QString rScript);
    void draw();


private:



    void createPlotControlPanel();
    void openPlotSettings();
    void loadSettings(QString fileName);
    void savePlotSettings();
    void savePlotSettingsAs();
    void createActions();
    void updateRecentRScriptsActs();
    void createToolBars();
    void importHomonyms(QDomDocument *settingsList);

    QScrollArea *plotControlPanelScrollArea;
    PlotSettingsForm *plotSettingsForm;
    QLabel *plotTitleLabel;
    QWidget *plotSettingsWidget;

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
    QString openPlotSettingsText;
    QString savePlotSettingsText;
    QString savePlotSettingsAsText;
    QMap <QString,QString> defaultSettings;

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

