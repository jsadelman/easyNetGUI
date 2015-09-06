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
class QComboBox;
//class ObjectCatalogueFilter;
class QGridLayout;

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
        void showPlotViewer();
public slots:
        void createNewPlotOfType(QString name, QString type,
                                 QMap<QString, QString> _defaultSettings=QMap<QString,QString>());
        void sendDrawCmd(QString plotName);
        void sendDrawCmd();
        void setPlot(QString name);
//        void hidePlotSettings();



private slots:
    void displaySVG(QByteArray plotByteArray, QString cmd);
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
//    void draw();
    void newAspectRatio(QSize);

    void buildWindow();
private:



    void createPlotControlPanel();
    void openPlotSettings();
    void loadSettings(QString fileName);
    void savePlotSettings();
    void savePlotSettingsAs();
    void createActions();
    void updateRecentRScriptsActs();
    void importHomonyms(QDomDocument *settingsList);

    QScrollArea *plotControlPanelScrollArea;
//    PlotSettingsForm *plotSettingsForm;
    QLabel *plotTitleLabel;
    QWidget *plotSettingsWidget;
//    QComboBox* plotNameBox;
    QLabel* plotNameBox;
    QLabel* plotTypeBox;

    QMap<QString, PlotSettingsForm*> plotForms;
    QMap<QString, QString> plotTypes;


    QMenu *typeMenu;
    QMenu *recentRScriptsMenu;
    LazyNutListMenu *dataMenu;
    LazyNutListMenu *plotsMenu;
//    QAction *drawAct;
    QAction *separatorAct;
    enum { MaxRecentRScripts = 5 };
    QAction *recentRScriptsActs[MaxRecentRScripts];
    QAction *selectRScriptAct;
    QAction *selectDataAct;
//    ObjectCatalogueFilter* plotListFilter;
    QString currentPlotType;
    QString currentOutput;
    QString currentPlot;
    QString createNewPlotText;
    QString openPlotSettingsText;
    QString savePlotSettingsText;
    QString savePlotSettingsAsText;
    QMap <QString,QString> defaultSettings;
    QGridLayout *gridLayout;
//    QVBoxLayout* vlayout;

    QString curFile;
    QMenu *fileMenu;
    QMenu *editMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *refreshAct;
    QAction *newPlotAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *copyAct;
    double plotAspr_;
};


#endif // PLOTWINDOW

