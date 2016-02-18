#ifndef PLOTSETTINGSWINDOW
#define PLOTSETTINGSWINDOW

#include <QMainWindow>
#include <QWizard>
#include <QMap>
#include <QSharedPointer>


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
class QGridLayout;

class NewPlotWizard: public QWizard
{
    Q_OBJECT

public:
    NewPlotWizard(QWidget *parent = 0);
    void accept() Q_DECL_OVERRIDE;

signals:
    void newRPlotCreated(QString, QString);

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
    QMap<QString, QString> getSettings(QString plotName);
signals:
        void plot(QString, QByteArray);
        void showPlotViewer();
        void newRPlotCreated(QString, bool, bool, QList<QSharedPointer<QDomDocument> >);
public slots:
        void newRPlot(QString name, QString type, QMap<QString, QString> defaultSettings=QMap<QString,QString>(),
                      int flags=0, QList<QSharedPointer<QDomDocument> > infoList=QList<QSharedPointer<QDomDocument> >());
        void sendGetCmd(QString plotName);
        void sendGetCmd();
        void setPlotSettings(QString name);
        void sendSettings(QString name = QString());
        void triggerRefresh();
        void sendDrawCmd(QString plotName);
        void refreshForm();

private slots:
    void displaySVG(QByteArray plotByteArray, QString cmd);
    void newPlot();
    void getSettingsXML(QString plotName);
    void buildSettingsForm(QString plotName, QDomDocument *domDoc,
                           QMap<QString, QString> defaultSettings=QMap<QString, QString>());
    void buildSettingsForm();
    void rebuildForm();

    void getPlotType();
    void extractPlotType(QDomDocument* description);
    void setCurrentPlotType(QString rScript);
    void newAspectRatio(QSize);
    void removePlotSettings(QString name);
    void setCurrentSettings(QDomDocument *settingsList) {currentSettings = settingsList;}
    void setCurrentPlotName(QString name) {currentPlotName = name;}

private:
     void buildWindow();
    void openPlotSettings();
    void loadSettings(QString fileName);
    void savePlotSettings();
    void savePlotSettingsAs();
    void createActions();
    void updateRecentRScriptsActs();
    void importHomonyms(QDomDocument *settingsList);

    QScrollArea *plotControlPanelScrollArea;
    QLabel *plotTitleLabel;
    QWidget *plotSettingsWidget;
    QLabel* plotNameBox;
    QLabel* plotTypeBox;

    QMap<QString, PlotSettingsForm*> plotForms;
    QMap<QString, QString> plotTypes;


    QMenu *typeMenu;
    QMenu *recentRScriptsMenu;
    QAction *separatorAct;
    enum { MaxRecentRScripts = 5 };
    QAction *recentRScriptsActs[MaxRecentRScripts];
    QAction *selectRScriptAct;
    QAction *selectDataAct;
    QString currentPlotType;
    QString currentOutput;
    QString currentPlotName;
    QDomDocument * currentSettings;
    QString createNewPlotText;
    QString openPlotSettingsText;
    QString savePlotSettingsText;
    QString savePlotSettingsAsText;
    QGridLayout *gridLayout;

    QString curFile;
    QMenu *fileMenu;
    QMenu *editMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *refreshAct;
    QAction *newPlotAct;
    QAction *plotAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *copyAct;
    double plotAspr_;
    bool quietly;
};


#endif // PLOTSETTINGSWINDOW

